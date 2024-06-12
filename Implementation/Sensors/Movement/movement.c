#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/button-hal.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "sys/log.h"

#include <stdio.h>
#include <stdlib.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

// CoAP server URL
#define COAP_SERVER_URL "coap://[fd00::1]:5683"
// Registration resource exposed by the CoAP server
#define REGISTRATION_RESOURCE "/register"

// Resource exposed by the current node
#define RESOURCE_NAME "movement"

// Maximum number of requests before sleeping
#define MAX_REQUESTS 5

// Sleep interval between a set of requests
#define SLEEP_INTERVAL 15*CLOCK_SECOND
static struct etimer sleep_timer;

extern coap_resource_t res_movement;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;

// Callback for the registration to the CoAP server
void client_chunk_handler(coap_message_t *response){
  
	if(response == NULL) {
		LOG_ERR("[Movement] Request timed out\n");
	}
  else if(response->code != 65){
		LOG_ERR("[Movement] Error: %d\n",response->code);	
	}
  else{
		LOG_INFO("[Movement] Registration successful\n");
		retry_requests = 0;		
		return;
	}
	
	retry_requests--;
	if(retry_requests==0)
		retry_requests = -1;
}

PROCESS(pir_motion_sensor_process, "PIR Motion Sensor Process");
AUTOSTART_PROCESSES(&pir_motion_sensor_process);

PROCESS_THREAD(pir_motion_sensor_process, ev, data)
{
  static button_hal_button_t *manual_opening;

  PROCESS_BEGIN();

  // Activate the resource exposed by the current node	
  coap_activate_resource(&res_movement, RESOURCE_NAME);
  
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

  manual_opening = button_hal_get_by_id(0);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event);
    if(ev == button_hal_press_event) {
      LOG_INFO("[Movement] Button pressed (%s)\n", BUTTON_HAL_GET_DESCRIPTION(manual_opening));
      res_movement.trigger();
    }

  }

  PROCESS_END();
}
