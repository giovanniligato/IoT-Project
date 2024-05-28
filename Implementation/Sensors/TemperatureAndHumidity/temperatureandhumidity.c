#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "os/dev/leds.h"
#include "json-senml.h"
#include "coap-observe-client.h"
#include "sys/clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define COAP_SERVER_URL "coap://[fd00::1]:5683"
#define REGISTRATION_RESOURCE "/register"
#define DISCOVERY_RESOURCE "/discovery"

#define VAULTSTATUS_RESOURCE "vaultstatus"
#define COAP_PORT 5683

#define RESOURCE_NAME "temperatureandhumidity"
#define MAX_REQUESTS 5

#define SLEEP_INTERVAL 15*CLOCK_SECOND

#define SAMPLE_INTERVAL 5*CLOCK_SECOND

#define LEDS_OFF 5

extern coap_resource_t res_temperatureandhumidity;
static struct etimer sleep_timer;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

static bool sleeping_mode = true;

extern bool hvac_status;

// Observe the resource 
static coap_observee_t *vaultstatus_resource;

PROCESS(temperatureandhumidity_sensor_process, "TemperatureAndHumidity sensor process");
AUTOSTART_PROCESSES(&temperatureandhumidity_sensor_process);

static void notification_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
{
  static senml_payload_t payload;
  static senml_measurement_t measurements[1];
  payload.measurements = measurements;
  payload.num_measurements = 1;

  const uint8_t *buffer = NULL;

  int buffer_size;
  if(notification){
    buffer_size = coap_get_payload(notification, &buffer);

  }

  switch (flag) {
    case NOTIFICATION_OK:
      
      LOG_DBG("NOTIFICATION RECEIVED in TemperatureAndHumidity Sensor: %s\n", buffer);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("ERROR in parsing the payload.\n");
        return;
      }

      // If all the leds are closed, the person is not in the room anymore -> sleeping mode is on
      // If the red led is open, hvac is on -> sleeping mode is off
      // If the green led is open, the hvac is off -> sleeping mode is off
      // If the yellow led is open, the person is waiting -> sleeping mode is off
      
      int led_value = (int) payload.measurements[0].value.v;
      sleeping_mode = false;
      hvac_status = false;
      if(led_value == LEDS_OFF)
        sleeping_mode = true;
      if(led_value == LEDS_CONF_RED)
        hvac_status = true;

      // printf("Sleeping mode: %d\n", sleeping_mode);

      if(!sleeping_mode){
        process_poll(&temperatureandhumidity_sensor_process);
      }

      break;

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("OBSERVE_OK\n");
      break;

    case ERROR_RESPONSE_CODE:
      printf("[TEMPERATURE] ERROR_RESPONSE_CODE: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[TEMPERATURE] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[TEMPERATURE] ERROR: Default in notification callback\n");
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


static char ip_vault_status[40];
static coap_endpoint_t coap_vault_status;

void resource_request_handler(coap_message_t *response){
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
    strncpy(ip_vault_status, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}


PROCESS_THREAD(temperatureandhumidity_sensor_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  coap_activate_resource(&res_temperatureandhumidity, RESOURCE_NAME);

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

  // Requesting the IP of the node where there is the vault status
  while(retry_requests!=0){

		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="VAULTSTATUS_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, resource_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  char coap_endpoint[100];
  snprintf(coap_endpoint, 100, "coap://[%s]:5683", ip_vault_status);  
  coap_endpoint_parse(coap_endpoint, strlen(coap_endpoint), &coap_vault_status);

  // Observing the vault status 
  vaultstatus_resource = coap_obs_request_registration(&coap_vault_status, "/"VAULTSTATUS_RESOURCE, notification_callback, NULL);

  // Imposta un timer per verificare lo stato del sensore ogni secondo
  etimer_set(&timer, SAMPLE_INTERVAL);

  while(1) {
    if(sleeping_mode){
      etimer_stop(&timer);
      LOG_DBG("I'm going to sleep\n");
      PROCESS_YIELD();
      etimer_restart(&timer);
    }
    else{
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      if(!sleeping_mode){
        res_temperatureandhumidity.trigger();
        etimer_reset(&timer);
      }
    }
  }

  // Stopping the observation
  coap_obs_remove_observee(vaultstatus_resource);

  PROCESS_END();
}
