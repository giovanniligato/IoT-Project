#include "contiki.h"
#include "coap-engine.h"
// #include "os/dev/button-hal.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "sys/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define COAP_SERVER_URL "coap://[fd00::1]:5683"
#define REGISTRATION_RESOURCE "/register"
#define DISCOVERY_RESOURCE "/discovery"

#define VAULT_STATUS_RESOURCE "vault_status"
#define COAP_PORT 5683

#define RESOURCE_NAME "co"
#define MAX_REQUESTS 5

#define SLEEP_INTERVAL 30*CLOCK_SECOND

#define SAMPLE_INTERVAL 5*CLOCK_SECOND


extern coap_resource_t res_co;
static struct etimer sleep_timer;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

static bool sleeping_mode = true;

// Observe the resource 
static coap_observee_t *vault_status_resource;

static void notification_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
{
  switch (flag) {
    case NOTIFICATION_OK:
      LOG_INFO("NOTIFICATION RECEIVED\n");
      sleeping_mode = !sleeping_mode;
      if(!sleeping_mode)
        PROCESS_POLL(&co_sensor_process);
      break;
    case OBSERVE_OK: /* server accepeted observation request */
      LOG_INFO("OBSERVE_OK\n");
      break;
    default: 
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

void resource_request_handler(coap_message_t *response){
	if(response == NULL) {
		LOG_ERR("Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("IP received successfully\n");
		retry_requests = 0;		
	
    coap_get_payload(response, &ip_vault_status);

		return;
	}

	retry_requests--;
	if(retry_requests==0)
		retry_requests=-1;
}


PROCESS(co_sensor_process, "CO sensor process");
AUTOSTART_PROCESSES(&co_sensor_process);

// Handler for the notifications sent by the vault status
void vault_status_handler(coap_message_t *response){
  sleeping_mode = !sleeping_mode;
  if(!sleeping_mode) // Prima il processo stava dormendo
    PROCESS_POLL(&co_sensor_process);
}

PROCESS_THREAD(co_sensor_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

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
    coap_set_header_uri_query(request, "requested_resource="VAULT_STATUS_RESOURCE);  
    
		COAP_BLOCKING_REQUEST(&coap_server, request, resource_request_handler);
    
		if(retry_requests == -1){		 
			etimer_set(&sleep_timer, SLEEP_INTERVAL);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			retry_requests = MAX_REQUESTS;
		}
	}

  // Observing the vault status
  vault_status_resource = coap_obs_request_registration(ip_vault_status, 
                                                        COAP_PORT,
                                                        "/"VAULT_STATUS_RESOURCE, 
                                                        notification_callback,
                                                        NULL);

  coap_activate_resource(&res_co, RESOURCE_NAME);

  // Imposta un timer per verificare lo stato del sensore ogni secondo
  etimer_set(&timer, SAMPLE_INTERVAL);

  while(1) {
    if(sleeping_mode)
      PROCESS_YIELD();
    else{
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      res_co.trigger();
      etimer_reset(&timer);
    }
  }

  // Stopping the observation
  coap_obs_remove_observee(vault_status_resource);


  PROCESS_END();
}


// INSERIRE VALORI CASUALI DENTRO RISORSA NEL TRIGGER
// FARE SERVER DISCOVERY 
// CERCARE DI COLLEGARE IL TUTTO
