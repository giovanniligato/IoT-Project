#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "os/dev/leds.h"
#include "json-senml.h"
#include "coap-observe-client.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

// CoAP server URL
#define COAP_SERVER_URL "coap://[fd00::1]:5683"
// Registration resource exposed by the CoAP server
#define REGISTRATION_RESOURCE "/register"
// Discovery resource exposed by the CoAP server
#define DISCOVERY_RESOURCE "/discovery"

// Resource exposed by the TemperatureAndHumidity sensor
#define TEMPERATUREANDHUMIDITY_RESOURCE "temperatureandhumidity"
// Resource exposed by the CO sensor
#define CO_RESOURCE "co"

// Resource exposed by the current node
#define RESOURCE_NAME "hvac"

// Maximum number of requests before sleeping
#define MAX_REQUESTS 5

// Sleep interval between a set of requests
#define SLEEP_INTERVAL 15*CLOCK_SECOND
static struct etimer sleep_timer;

extern coap_resource_t res_hvac;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

// Observe the temperatureandhumidity resource 
static coap_observee_t *temperatureandhumidity_resource;
// Observe the co resource
static coap_observee_t *co_resource;

double current_temperature = -1.0;
double current_humidity = -1.0;
double current_co = -1.0;

static bool temperature_received = false;
static bool humidity_received = false;
static bool co_received = false;

// Used to prevent stalls when one 
// sensor is in sleep mode while the 
// other sensor continues sending data, 
// even though it should also be in sleep mode.
#define MAX_DETECTOR_SENSOR_OFF 3
static int detector_sensor_off = 0;

PROCESS(hvac_process, "HVAC process");
AUTOSTART_PROCESSES(&hvac_process);

// Callback for the CO sensor
static void co_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
{
  static senml_payload_t payload;
  
  static senml_measurement_t measurements[1];
  payload.measurements = measurements;
  payload.num_measurements = 1;

  static char base_name[MAX_STRING_LEN];
  static char name[1][MAX_STRING_LEN];
  static char unit[1][MAX_STRING_LEN];
  
  payload.base_name = base_name;
  payload.measurements[0].name = name[0];
  payload.measurements[0].unit = unit[0];

  const uint8_t *buffer = NULL;

  int buffer_size = 0;
  if(notification){
    buffer_size = coap_get_payload(notification, &buffer);
  }

  switch (flag) {
    case NOTIFICATION_OK:

      LOG_DBG("[HVAC] Notification received from CO sensor: %s\n", buffer);
      LOG_DBG("[HVAC] In co_callback payload.num_measurements is: %d\n", payload.num_measurements);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("[HVAC] ERROR in parsing the payload.\n");
        return;
      }

      current_co = payload.measurements[0].value.v;
      co_received = true;

      if((co_received && temperature_received && humidity_received) || (detector_sensor_off >= MAX_DETECTOR_SENSOR_OFF)){
        res_hvac.trigger();
        co_received = temperature_received = humidity_received = false;
        detector_sensor_off = 0;
      }
      else{
        detector_sensor_off++;
      }

      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("[HVAC] OBSERVE_OK from CO sensor\n");
      break;
    
    case ERROR_RESPONSE_CODE:
      printf("[HVAC] ERROR_RESPONSE_CODE from CO sensor: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[HVAC] NO_REPLY_FROM_SERVER from CO sensor: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;

    default: 
      LOG_ERR("[HVAC] ERROR from CO sensor: Default in notification callback\n");
      break;

  }

}

// Callback for the TemperatureAndHumidity sensor
static void temperatureandhumidity_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
{
  static senml_payload_t payload;
  static senml_measurement_t measurements[2];
  payload.measurements = measurements;
  payload.num_measurements = 2;

  static char base_name[MAX_STRING_LEN];
  static char name[2][MAX_STRING_LEN];
  static char unit[2][MAX_STRING_LEN];
  
  payload.base_name = base_name;
  payload.measurements[0].name = name[0];
  payload.measurements[0].unit = unit[0];

  payload.measurements[1].name = name[1];
  payload.measurements[1].unit = unit[1];

 
  const uint8_t *buffer = NULL;

  int buffer_size = 0;
  if(notification){
    buffer_size = coap_get_payload(notification, &buffer);
  }

  switch (flag) {
    case NOTIFICATION_OK:

      LOG_DBG("[HVAC] Notification received from TemperatureAndHumidity sensor: %s\n", buffer);

      LOG_DBG("[HVAC] In temperatureandhumidity_callback payload.num_measurements is: %d\n", payload.num_measurements);
      
      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("[HVAC] ERROR in parsing the payload.\n");
        return;
      } 

      for(int i = 0; i < payload.num_measurements; i++){
        if(strcmp(payload.measurements[i].name, "temperature") == 0){
          current_temperature = payload.measurements[i].value.v;
          temperature_received = true;
        }
        else if(strcmp(payload.measurements[i].name, "humidity") == 0){
          current_humidity = payload.measurements[i].value.v;
          humidity_received = true;
        }
      }

      if((co_received && temperature_received && humidity_received) || (detector_sensor_off >= MAX_DETECTOR_SENSOR_OFF)){
        res_hvac.trigger();
        co_received = temperature_received = humidity_received = false;
        detector_sensor_off = 0;
      }
      else{
        detector_sensor_off++;
      }

      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("[HVAC] OBSERVE_OK from TemperatureAndHumidity sensor\n");
      
      break;

    case ERROR_RESPONSE_CODE:
      printf("[HVAC] ERROR_RESPONSE_CODE from TemperatureAndHumidity sensor: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[HVAC] NO_REPLY_FROM_SERVER from TemperatureAndHumidity sensor: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[HVAC] ERROR from TemperatureAndHumidity sensor: Default in notification callback\n");
      break;

  }

}

// Callback for the registration to the CoAP server
void client_chunk_handler(coap_message_t *response){
  
	if(response == NULL) {
		LOG_ERR("[HVAC] Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("[HVAC] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[HVAC] Registration successful\n");
		retry_requests = 0;		
		return;
	}
	
	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}

// IP of the node where the CO sensor is located
static char ip_co[40];
// CoAP endpoint of the node where the CO sensor is located
static coap_endpoint_t coap_co;

// IP of the node where the TemperatureAndHumidity sensor is located
static char ip_temperatureandhumidity[40];
// CoAP endpoint of the node where the TemperatureAndHumidity sensor is located
static coap_endpoint_t coap_temperatureandhumidity;


// Callback for the request of the IP of the node where the CO sensor is located
void co_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("[HVAC] Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("[HVAC] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[HVAC] IP of the CO sensor received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_co, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}

// Callback for the request of the IP of the node where the TemperatureAndHumidity sensor is located
void temperatureandhumidity_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("[HVAC] Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("[HVAC] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[HVAC] IP of the TemperatureAndHumidity sensor received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_temperatureandhumidity, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}


PROCESS_THREAD(hvac_process, ev, data)
{
  PROCESS_BEGIN();
  
  // Activate the resource exposed by the current node
  coap_activate_resource(&res_hvac, RESOURCE_NAME);

  // Registration to the CoAP server
  while(retry_requests!=0){

    // Parsing the CoAP server URL
		coap_endpoint_parse(COAP_SERVER_URL, strlen(COAP_SERVER_URL), &coap_server);
    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
		coap_set_header_uri_path(request, REGISTRATION_RESOURCE);
    // Setting the payload of the request
		coap_set_payload(request, (uint8_t *)RESOURCE_NAME, sizeof(RESOURCE_NAME) - 1);
	
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, client_chunk_handler);
    
		if(retry_requests == -1){
      // If the maximum number of requests has been reached, sleep for a while
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  retry_requests = MAX_REQUESTS;
  // Requesting the IP of the node where there is the TemperatureAndHumidity sensor
  while(retry_requests!=0){

    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="TEMPERATUREANDHUMIDITY_RESOURCE);  
    
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, temperatureandhumidity_request_handler);
    
		if(retry_requests == -1){	
      // If the maximum number of requests has been reached, the node goes to sleep	 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  // CoAP endpoint of the TemperatureAndHumidity sensor
  char coap_temperatureandhumidity_endpoint[100];
  snprintf(coap_temperatureandhumidity_endpoint, 100, "coap://[%s]:5683", ip_temperatureandhumidity);  
  coap_endpoint_parse(coap_temperatureandhumidity_endpoint, strlen(coap_temperatureandhumidity_endpoint), &coap_temperatureandhumidity);

  // Observing the TemperatureAndHumidity sensor 
  temperatureandhumidity_resource = coap_obs_request_registration(&coap_temperatureandhumidity, TEMPERATUREANDHUMIDITY_RESOURCE, temperatureandhumidity_callback, NULL);


  retry_requests = MAX_REQUESTS;
  // Requesting the IP of the node where there is the CO sensor
  while(retry_requests!=0){

    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="CO_RESOURCE);  
    
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, co_request_handler);
    
		if(retry_requests == -1){
      // If the maximum number of requests has been reached, the node goes to sleep		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}
  
  // CoAP endpoint of the CO sensor
  char coap_co_endpoint[100];
  snprintf(coap_co_endpoint, 100, "coap://[%s]:5683", ip_co);  
  coap_endpoint_parse(coap_co_endpoint, strlen(coap_co_endpoint), &coap_co);

  // Observing the CO sensor 
  co_resource = coap_obs_request_registration(&coap_co, CO_RESOURCE, co_callback, NULL);

  while(1) {
    PROCESS_YIELD();
  }

  // Stopping the observation
  coap_obs_remove_observee(temperatureandhumidity_resource);
  coap_obs_remove_observee(co_resource);

  PROCESS_END();
}
