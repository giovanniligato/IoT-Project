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

// Resource exposed by the Movement sensor
#define MOVEMENT_RESOURCE "movement"
// Resource exposed by the HVAC
#define HVAC_RESOURCE "hvac"

// Resource exposed by the current node
#define RESOURCE_NAME "vaultstatus"

// Maximum number of requests before sleeping
#define MAX_REQUESTS 5

// Sleep interval between a set of requests
#define SLEEP_INTERVAL 15*CLOCK_SECOND
static struct etimer sleep_timer;

// Interval during which the automatic door is open
#define OPEN_AUTOMATIC_DOOR_SECONDS 6

// All leds off
#define ALL_LEDS_OFF 5

extern coap_resource_t res_vaultstatus;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

// Observe the movement resource 
static coap_observee_t *movement_resource;
// Observe the hvac resource
static coap_observee_t *hvac_resource;

// Status of the leds
unsigned int led_status = ALL_LEDS_OFF;

// Automatic door timer
static struct etimer automatic_door_timer;

PROCESS(vaultstatus_process, "VaultStatus process");
AUTOSTART_PROCESSES(&vaultstatus_process);

// Callback for the Movement sensor
static void movement_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
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

      // From movement we receive the boolean vault_activated

      LOG_DBG("[VaultStatus] Notification received from Movement sensor: %s\n", buffer);

      LOG_DBG("[VaultStatus] In movement_callback payload.num_measurements is: %d\n", payload.num_measurements);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("[VaultStatus] ERROR in parsing the payload.\n");
        return;
      }

      // If all LEDs are off, the human operator is no longer in the room -> sleep mode is on
      // If the red LED is on, HVAC is active -> sleep mode is off
      // If the green LED is on, HVAC is inactive -> sleep mode is off
      // If the yellow LED is on, the human operator is waiting -> sleep mode is off

      if(payload.measurements[0].value.bv){
        // vault_activated is true
        leds_single_on(LEDS_YELLOW);
        led_status = LEDS_YELLOW;
      }
      else{
        // vault_activated is false
        leds_single_off(LEDS_YELLOW);        
        leds_off(LEDS_ALL);
        
        if(led_status == LEDS_GREEN){
          // The human operator is leaving the room
          LOG_DBG("[VaultStatus] Opening the automatic door\n");
          process_poll(&vaultstatus_process);
        }
        led_status = ALL_LEDS_OFF;
      }

      // Trigger the notification of the vaultstatus resource
      res_vaultstatus.trigger();
      
      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("[VaultStatus] OBSERVE_OK from Movement sensor\n");   
      break;

    case ERROR_RESPONSE_CODE:
      printf("[VaultStatus] ERROR_RESPONSE_CODE from Movement sensor: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[VaultStatus] NO_REPLY_FROM_SERVER from Movement sensor: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      
    default: 
      LOG_ERR("[VaultStatus] ERROR from Movement sensor: Default in notification callback\n");
      break;

  }

}

// Callback for the HVAC
static void hvac_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
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

  unsigned int old_led_status = led_status;

  int buffer_size = 0;
  if(notification){
    buffer_size = coap_get_payload(notification, &buffer);
  }

  switch (flag) {
    case NOTIFICATION_OK:

      // From HVAC we receive the boolean hvac_status
      LOG_DBG("[VaultStatus] Notification received from HVAC: %s\n", buffer);

      LOG_DBG("In hvac_callback payload.num_measurements is: %d\n", payload.num_measurements);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("[VaultStatus] ERROR in parsing the payload.\n");
        return;
      }

      // If the HVAC is on, the red LED is on
      // If the HVAC is off, the green LED is on

      if(led_status != ALL_LEDS_OFF){
        // The vault is activated

        if(!payload.measurements[0].value.bv){
          // hvac_status is false
          leds_single_off(LEDS_YELLOW);        
          leds_off(LEDS_ALL);
          #ifdef COOJA
            leds_single_on(LEDS_GREEN);
          #else
            leds_on(LEDS_GREEN);
          #endif
          led_status = LEDS_GREEN;
        }
        else{
          // hvac_status is true
          leds_single_off(LEDS_YELLOW);        
          leds_off(LEDS_ALL);
          #ifdef COOJA
            leds_single_on(LEDS_RED);
          #else
            leds_on(LEDS_RED);
          #endif
          led_status = LEDS_RED;
        }

      }
      
      // Trigger the notification of the vaultstatus resource
      res_vaultstatus.trigger();

      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("[VaultStatus] OBSERVE_OK from HVAC\n");
      
      break;

    case ERROR_RESPONSE_CODE:
      printf("[VaultStatus] ERROR_RESPONSE_CODE from HVAC: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[VaultStatus] NO_REPLY_FROM_SERVER from HVAC: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[VaultStatus] ERROR from HVAC: Default in notification callback\n");
      break;

  }

  if(led_status == LEDS_GREEN && old_led_status != LEDS_GREEN){
    LOG_DBG("[VaultStatus] Opening the automatic door\n");
    process_poll(&vaultstatus_process);
  }

}

// Callback for the registration to the CoAP server
void client_chunk_handler(coap_message_t *response){
  
	if(response == NULL) {
		LOG_ERR("[VaultStatus] Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("[VaultStatus] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[VaultStatus] Registration successful\n");
		retry_requests = 0;		
		return;
	}
	
	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}


// IP of the node where the Movement sensor is located
static char ip_movement[40];
// CoAP endpoint of the node where the Movement sensor is located
static coap_endpoint_t coap_movement;

// IP of the node where the HVAC is located
static char ip_hvac[40];
// CoAP endpoint of the node where the HVAC is located
static coap_endpoint_t coap_hvac;

// Callback for the request of the IP of the node where the Movement sensor is located
void movement_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("[VaultStatus] Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("[VaultStatus] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[VaultStatus] IP of the Movement sensor received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_movement, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}

// Callback for the request of the IP of the node where the HVAC is located
void hvac_request_handler(coap_message_t *response){
  const uint8_t *buffer = NULL;

	if(response == NULL) {
		LOG_ERR("[VaultStatus] Request timed out\n");
	}
  else if(response->code != 69){
		LOG_ERR("[VaultStatus] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[VaultStatus] IP of the HVAC received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &buffer);
    strncpy(ip_hvac, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}


PROCESS_THREAD(vaultstatus_process, ev, data)
{
  static int i;

  PROCESS_BEGIN();

  // Activate the resource exposed by the current node
  coap_activate_resource(&res_vaultstatus, RESOURCE_NAME);

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
  // Requesting the IP of the Movement sensor node
  while(retry_requests!=0){

    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="MOVEMENT_RESOURCE);  
    
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, movement_request_handler);
    
		if(retry_requests == -1){
      // If the maximum number of requests has been reached, sleep for a while
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  // CoAP endpoint of the Movement sensor
  char coap_movement_endpoint[100];
  snprintf(coap_movement_endpoint, 100, "coap://[%s]:5683", ip_movement);  
  coap_endpoint_parse(coap_movement_endpoint, strlen(coap_movement_endpoint), &coap_movement);

  // Observing the Movement sensor
  movement_resource = coap_obs_request_registration(&coap_movement, MOVEMENT_RESOURCE, movement_callback, NULL);

  retry_requests = MAX_REQUESTS;
  // Requesting the IP of the HVAC node
  while(retry_requests!=0){

    // Initializing the request
		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="HVAC_RESOURCE);  
    
    // Sending the request
		COAP_BLOCKING_REQUEST(&coap_server, request, hvac_request_handler);
    
		if(retry_requests == -1){		
      // If the maximum number of requests has been reached, sleep for a while 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  // CoAP endpoint of the HVAC
  char coap_hvac_endpoint[100];
  snprintf(coap_hvac_endpoint, 100, "coap://[%s]:5683", ip_hvac);  
  coap_endpoint_parse(coap_hvac_endpoint, strlen(coap_hvac_endpoint), &coap_hvac);

  // Observing the HVAC
  hvac_resource = coap_obs_request_registration(&coap_hvac, HVAC_RESOURCE, hvac_callback, NULL);

  etimer_set(&automatic_door_timer, CLOCK_SECOND);

  while(1) {

    LOG_DBG("[VaultStatus] Process going to sleep\n");
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&automatic_door_timer));
    PROCESS_YIELD();
    LOG_DBG("[VaultStatus] Process woke up\n");
    etimer_restart(&automatic_door_timer);
    for(i = 0; i < OPEN_AUTOMATIC_DOOR_SECONDS; i++){
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&automatic_door_timer));
      #ifdef COOJA
        leds_single_toggle(LEDS_GREEN);
      #else
        leds_toggle(LEDS_GREEN);
      #endif
      etimer_reset(&automatic_door_timer);
    }      
    
  }

  // Stopping the observation
  coap_obs_remove_observee(movement_resource);
  coap_obs_remove_observee(hvac_resource);

  PROCESS_END();
}

