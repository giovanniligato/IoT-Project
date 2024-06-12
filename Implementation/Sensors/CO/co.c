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

// CoAP server URL
#define COAP_SERVER_URL "coap://[fd00::1]:5683"
// Registration resource exposed by the CoAP server
#define REGISTRATION_RESOURCE "/register"
// Discovery resource exposed by the CoAP server
#define DISCOVERY_RESOURCE "/discovery"

// Resource exposed by the VaultStatus
#define VAULTSTATUS_RESOURCE "vaultstatus"

// Resource exposed by the current node
#define RESOURCE_NAME "co"

// Maximum number of requests before sleeping
#define MAX_REQUESTS 5

// Sleep interval between a set of requests
#define SLEEP_INTERVAL 15*CLOCK_SECOND
static struct etimer sleep_timer;

// Sample interval for the CO sensor
#define SAMPLE_INTERVAL 11*CLOCK_SECOND

// All LEDs off
#define ALL_LEDS_OFF 5

extern coap_resource_t res_co;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

static bool sleeping_mode = true;

bool hvac_status = false;

// Observe the vaultstatus resource
static coap_observee_t *vaultstatus_resource;

PROCESS(co_sensor_process, "CO sensor process");
AUTOSTART_PROCESSES(&co_sensor_process);

// Callback for the VaultStatus
static void notification_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
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

      LOG_DBG("[CO] Notification received from VaultStatus: %s\n", buffer);
      
      LOG_DBG("[CO] In notification_callback payload.num_measurements is: %d\n", payload.num_measurements);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("[CO] ERROR in parsing the payload.\n");
        return;
      } 

      // If all LEDs are off, the human operator is no longer in the room -> sleep mode is on
      // If the red LED is on, HVAC is active -> sleep mode is off
      // If the green LED is on, HVAC is inactive -> sleep mode is off
      // If the yellow LED is on, the human operator is waiting -> sleep mode is off
      
      int led_value = (int) payload.measurements[0].value.v;
      sleeping_mode = false;
      hvac_status = false;
      if(led_value == ALL_LEDS_OFF)
        sleeping_mode = true;
      if(led_value == LEDS_RED)
        hvac_status = true;

      if(!sleeping_mode){
        // Wake up the CO sensor
        process_poll(&co_sensor_process);
      }

      break;

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("[CO] OBSERVE_OK from VaultStatus\n");
      break;

    case ERROR_RESPONSE_CODE:
      printf("[CO] ERROR_RESPONSE_CODE from VaultStatus: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[CO] NO_REPLY_FROM_SERVER from VaultStatus: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      
    default: 
      LOG_ERR("[CO] ERROR from VaultStatus: Default in notification callback\n");
      break;
      
  }

}

// Callback for the registration to the CoAP server
void client_chunk_handler(coap_message_t *response){
  
	if(response == NULL) {
		LOG_ERR("[CO] Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("[CO] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[CO] Registration successful\n");
		retry_requests = 0;		
		return;
	}
	
	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}

// IP of the node where the VaultStatus is located
static char ip_vault_status[40];
// CoAP endpoint of the node where the VaultStatus is located
static coap_endpoint_t coap_vault_status;

// Callback for the request of the IP of the node where the VaultStatus is located
void resource_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("[CO] Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("[CO] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[CO] IP of the VaultStatus received successfully\n");
		retry_requests = 0;
	
    coap_get_payload(response, &buffer);
    strncpy(ip_vault_status, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}


PROCESS_THREAD(co_sensor_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  // Activate the resource exposed by the current node
  coap_activate_resource(&res_co, RESOURCE_NAME);

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
  // Requesting the IP of the node where there is the VaultStatus
  while(retry_requests!=0){

    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="VAULTSTATUS_RESOURCE);  
    
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, resource_request_handler);
    
		if(retry_requests == -1){
      // If the maximum number of requests has been reached, sleep for a while
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  // CoAP endpoint of the VaultStatus
  char coap_endpoint[100];
  snprintf(coap_endpoint, 100, "coap://[%s]:5683", ip_vault_status);  
  coap_endpoint_parse(coap_endpoint, strlen(coap_endpoint), &coap_vault_status);

  // Observing the VaultStatus
  vaultstatus_resource = coap_obs_request_registration(&coap_vault_status, VAULTSTATUS_RESOURCE, notification_callback, NULL);

  // Initializing the sampling timer
  etimer_set(&timer, SAMPLE_INTERVAL);

  while(1) {

    if(sleeping_mode){
      // If the node is in sleep mode, it sleeps
      etimer_stop(&timer);
      LOG_DBG("[CO] I'm going to sleep\n");
      PROCESS_YIELD();
      etimer_restart(&timer);
    }
    else{
      // If the node is not in sleep mode, it samples the co level
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      if(!sleeping_mode){
        res_co.trigger();
        etimer_reset(&timer);
      }
    }

  }

  // Stopping the observation
  coap_obs_remove_observee(vaultstatus_resource);

  PROCESS_END();
}
