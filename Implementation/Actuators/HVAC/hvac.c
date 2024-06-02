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

#define COAP_SERVER_URL "coap://[fd00::1]:5683"
#define REGISTRATION_RESOURCE "/register"
#define DISCOVERY_RESOURCE "/discovery"

#define TEMPERATUREANDHUMIDITY_RESOURCE "temperatureandhumidity"
#define CO_RESOURCE "co"
#define COAP_PORT 5683

#define RESOURCE_NAME "hvac"
#define MAX_REQUESTS 5

#define SLEEP_INTERVAL 15*CLOCK_SECOND

extern coap_resource_t res_hvac;
static struct etimer sleep_timer;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

// Observe the temperatureandhumidity resource 
static coap_observee_t *co_resource;
// Observe the co resource
static coap_observee_t *temperatureandhumidity_resource;

double current_temperature = -1.0;
double current_humidity = -1.0;
double current_co = -1.0;

static bool temperature_received = false;
static bool humidity_received = false;
static bool co_received = false;

#define MAX_DETECTOR_SENSOR_OFF 3
static int detector_sensor_off = 0;

PROCESS(hvac_process, "HVAC process");
AUTOSTART_PROCESSES(&hvac_process);

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

      // Da movement riceviamo il booleano vault_activated

      LOG_DBG("NOTIFICATION RECEIVED in HVAC by CO sensor: %s\n", buffer);

      LOG_DBG("In co_callback payload.num_measurements is: %d\n", payload.num_measurements);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("ERROR in parsing the payload.\n");
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
      LOG_INFO("OBSERVE_OK\n");
      
      break;
    
    case ERROR_RESPONSE_CODE:
      printf("[HVAC-CO] ERROR_RESPONSE_CODE: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[HVAC-CO] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[HVAC-CO] ERROR: Default in notification callback\n");
      break;

  }

}

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

      // Da movement riceviamo il booleano vault_activated

      LOG_DBG("NOTIFICATION RECEIVED in HVAC by TemperatureAndHumidity sensor: %s\n", buffer);

      LOG_DBG("In temperatureandhumidity_callback payload.num_measurements is: %d\n", payload.num_measurements);
      
      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("ERROR in parsing the payload.\n");
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
      LOG_INFO("OBSERVE_OK\n");
      
      break;

    case ERROR_RESPONSE_CODE:
      printf("[HVAC-Temp] ERROR_RESPONSE_CODE: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[HVAC-Temp] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[HVAC-Temp] ERROR: Default in notification callback\n");
      break;

  }

}

void client_chunk_handler(coap_message_t *response){
  
	if(response == NULL) {
		LOG_ERR("Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("Registration successful\n");
		retry_requests = 0;		
		return;
	}
	
	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}


static char ip_co[40];
static coap_endpoint_t coap_co;

static char ip_temperatureandhumidity[40];
static coap_endpoint_t coap_temperatureandhumidity;

void co_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("IP received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_co, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}

void temperatureandhumidity_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("IP received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_temperatureandhumidity, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}


PROCESS_THREAD(hvac_process, ev, data)
{
  PROCESS_BEGIN();
  
  coap_activate_resource(&res_hvac, RESOURCE_NAME);

  while(retry_requests!=0){

    // Registration to the CoAP server
		coap_endpoint_parse(COAP_SERVER_URL, strlen(COAP_SERVER_URL), &coap_server);
		coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
		coap_set_header_uri_path(request, REGISTRATION_RESOURCE);
		coap_set_payload(request, (uint8_t *)RESOURCE_NAME, sizeof(RESOURCE_NAME) - 1);
	
		COAP_BLOCKING_REQUEST(&coap_server, request, client_chunk_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  retry_requests = MAX_REQUESTS;

  // Requesting the IP of the node where there is the temperatureandhumidity sensor
  while(retry_requests!=0){

		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="TEMPERATUREANDHUMIDITY_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, temperatureandhumidity_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  char coap_temperatureandhumidity_endpoint[100];
  snprintf(coap_temperatureandhumidity_endpoint, 100, "coap://[%s]:5683", ip_temperatureandhumidity);  
  coap_endpoint_parse(coap_temperatureandhumidity_endpoint, strlen(coap_temperatureandhumidity_endpoint), &coap_temperatureandhumidity);

  // Observing the temperatureandhumidity sensor 
  temperatureandhumidity_resource = coap_obs_request_registration(&coap_temperatureandhumidity, TEMPERATUREANDHUMIDITY_RESOURCE, temperatureandhumidity_callback, NULL);


  retry_requests = MAX_REQUESTS;

  // Requesting the IP of the node where there is the co sensor
  while(retry_requests!=0){

		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="CO_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, co_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}
  

  char coap_co_endpoint[100];
  snprintf(coap_co_endpoint, 100, "coap://[%s]:5683", ip_co);  
  coap_endpoint_parse(coap_co_endpoint, strlen(coap_co_endpoint), &coap_co);

  // Observing the co sensor 
  co_resource = coap_obs_request_registration(&coap_co, CO_RESOURCE, co_callback, NULL);

  while(1) {
    PROCESS_YIELD();
  }

  // Stopping the observation
  coap_obs_remove_observee(temperatureandhumidity_resource);
  coap_obs_remove_observee(co_resource);

  PROCESS_END();
}

