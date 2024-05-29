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

#define MOVEMENT_RESOURCE "movement"
#define HVAC_RESOURCE "hvac"
#define COAP_PORT 5683

#define RESOURCE_NAME "vaultstatus"
#define MAX_REQUESTS 5

#define SLEEP_INTERVAL 15*CLOCK_SECOND

#define OPEN_AUTOMATIC_DOOR_SECONDS 4

#define LEDS_OFF 5

extern coap_resource_t res_vaultstatus;
static struct etimer sleep_timer;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

// Observe the movement resource 
static coap_observee_t *movement_resource;

// Observe the hvac resource
static coap_observee_t *hvac_resource;

unsigned int led_status = LEDS_OFF;

static struct etimer automatic_door_timer;

PROCESS(vaultstatus_process, "Vault Status process");
AUTOSTART_PROCESSES(&vaultstatus_process);

static void movement_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
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

      // Da movement riceviamo il booleano vault_activated

      LOG_DBG("NOTIFICATION RECEIVED in VaultStatus: %s\n", buffer);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("ERROR in parsing the payload.\n");
        return;
      }

      // If all the leds are closed, the person is not in the room anymore -> sleeping mode is on
      // If the red led is open, hvac is on -> sleeping mode is off
      // If the green led is open, the hvac is off -> sleeping mode is off
      // If the yellow led is open, the person is waiting -> sleeping mode is off
      
      if(payload.measurements[0].value.bv){
        leds_single_on(LEDS_YELLOW);
        led_status = LEDS_CONF_YELLOW;
      }
      else{
        if(led_status == LEDS_GREEN){
          // The human operator is leaving the room
          // fai lampeggiare per 15 secondi ad intervalli di un secondo
          process_poll(&vaultstatus_process);
        }
        leds_single_off(LEDS_YELLOW);        
        leds_off(LEDS_ALL);
        led_status = LEDS_OFF;
      }

      // Trigger the notification of the vault status resource
      res_vaultstatus.trigger();
      
      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("OBSERVE_OK\n");
      
      break;

    case ERROR_RESPONSE_CODE:
      printf("[VaultStatus] ERROR_RESPONSE_CODE: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[VaultStatus] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[VaultStatus] ERROR: Default in notification callback\n");
      break;

  }

}

static void hvac_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
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

      // Da hvac riceviamo il booleano hvac_status

      LOG_DBG("NOTIFICATION RECEIVED in VaultStatus by HVAC: %s\n", buffer);

      if(parse_senml_payload((char*)buffer, buffer_size, &payload) == -1){
        LOG_ERR("ERROR in parsing the payload.\n");
        return;
      }

      // If the hvac is on, the red led is on
      // If the hvac is off, the green led is on

      if(payload.measurements[0].value.bv){
        leds_single_off(LEDS_YELLOW);        
        leds_off(LEDS_ALL);
        leds_on(LEDS_GREEN);
        led_status = LEDS_GREEN;
        // Fare lampeggiare led verde per 5 secondi
      }
      else{
        leds_single_off(LEDS_YELLOW);        
        leds_off(LEDS_ALL);
        leds_on(LEDS_RED);
        led_status = LEDS_RED;
      }

      // Trigger the notification of the vault status resource
      res_vaultstatus.trigger();
      
      break;        

    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("OBSERVE_OK\n");
      
      break;

    case ERROR_RESPONSE_CODE:
      printf("[VaultStatus] ERROR_RESPONSE_CODE: %*s\n", buffer_size, (char *)buffer);
      obs = NULL;
      break;
    case NO_REPLY_FROM_SERVER:
      printf("[VaultStatus] NO_REPLY_FROM_SERVER: "
            "removing observe registration with token %x%x\n",
            obs->token[0], obs->token[1]);
      obs = NULL;
      break;
      

    default: 
      LOG_ERR("[VaultStatus] ERROR: Default in notification callback\n");
      break;

  }

  if(led_status == LEDS_GREEN){
    // fai lampeggiare per 5 secondi ad intervalli di un secondo
    process_poll(&vaultstatus_process);
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


static char ip_movement[40];
static coap_endpoint_t coap_movement;

static char ip_hvac[40];
static coap_endpoint_t coap_hvac;


void movement_request_handler(coap_message_t *response){
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
    strncpy(ip_movement, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}

void hvac_request_handler(coap_message_t *response){
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
    strncpy(ip_hvac, (char *)buffer, response->payload_len);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}


PROCESS_THREAD(vaultstatus_process, ev, data)
{
  PROCESS_BEGIN();
  
  coap_activate_resource(&res_vaultstatus, RESOURCE_NAME);

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

  // Requesting the IP of the movement sensor node
  while(retry_requests!=0){

		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="MOVEMENT_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, movement_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  char coap_movement_endpoint[100];
  snprintf(coap_movement_endpoint, 100, "coap://[%s]:5683", ip_movement);  
  coap_endpoint_parse(coap_movement_endpoint, strlen(coap_movement_endpoint), &coap_movement);

  // Observing the movement sensor
  movement_resource = coap_obs_request_registration(&coap_movement, MOVEMENT_RESOURCE, movement_callback, NULL);


  retry_requests = MAX_REQUESTS;

  // Requesting the IP of the HVAC node
  while(retry_requests!=0){

		coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
		coap_set_header_uri_path(request, DISCOVERY_RESOURCE);
    // Add uri query to the request
    coap_set_header_uri_query(request, "requested_resource="HVAC_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, hvac_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}


  char coap_hvac_endpoint[100];
  snprintf(coap_hvac_endpoint, 100, "coap://[%s]:5683", ip_hvac);  
  coap_endpoint_parse(coap_hvac_endpoint, strlen(coap_hvac_endpoint), &coap_hvac);

  // Observing the hvac
  hvac_resource = coap_obs_request_registration(&coap_hvac, HVAC_RESOURCE, hvac_callback, NULL);



  etimer_set(&automatic_door_timer, CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();
    
    etimer_restart(&automatic_door_timer);
    for(int i = 0; i < OPEN_AUTOMATIC_DOOR_SECONDS; i++){
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&automatic_door_timer));
      leds_toggle(LEDS_GREEN);
      etimer_reset(&automatic_door_timer);      
    }      
    
  }

  // Stopping the observation
  coap_obs_remove_observee(movement_resource);

  PROCESS_END();
}

