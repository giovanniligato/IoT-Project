#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/button-hal.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "sys/log.h"

#ifndef COOJA
#include "gpio-hal-arch.h"
#include "nrfx_gpiote.h"
#include "nrfx_rtc.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define PIR_SENSOR_PORT 0  // Port 0
#define PIR_SENSOR_PIN  29 // Pin 29


#define COAP_SERVER_URL "coap://[fd00::1]:5683"
#define REGISTRATION_RESOURCE "/register"

#define RESOURCE_NAME "movement"
#define MAX_REQUESTS 5

#define SLEEP_INTERVAL 30*CLOCK_SECOND

extern coap_resource_t res_movement;
static struct etimer sleep_timer;

static coap_endpoint_t coap_server;
static coap_message_t request[1];       
static int retry_requests = MAX_REQUESTS;


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

PROCESS(pir_motion_sensor_process, "PIR Motion Sensor Process");
AUTOSTART_PROCESSES(&pir_motion_sensor_process);

PROCESS_THREAD(pir_motion_sensor_process, ev, data)
{
  static struct etimer timer;
  static button_hal_button_t *manual_opening;

  PROCESS_BEGIN();

  #ifndef COOJA
  // Inizializza il driver GPIOTE se non è già inizializzato
  if(!nrfx_gpiote_is_init()) {
    nrfx_gpiote_init();
  }

  // Configura il pin del sensore PIR come input
  gpio_hal_arch_pin_set_input(PIR_SENSOR_PORT, PIR_SENSOR_PIN);
  #endif
  	
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

  coap_activate_resource(&res_movement, RESOURCE_NAME);

  // Imposta un timer per verificare lo stato del sensore ogni secondo
  etimer_set(&timer, CLOCK_SECOND);

  manual_opening = button_hal_get_by_id(0);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer) || (ev == button_hal_press_event));
    if(ev == button_hal_press_event) {
      LOG_INFO("Button pressed (%s)\n", BUTTON_HAL_GET_DESCRIPTION(manual_opening));
      res_movement.trigger();
    }
    #ifndef COOJA
    else if(nrf_gpio_pin_read(NRF_GPIO_PIN_MAP(PIR_SENSOR_PORT, PIR_SENSOR_PIN))) { // Legge lo stato del pin del sensore PIR
      // Chiamata a una funzione per notificare il movimento (trigger)
      res_movement.trigger();
    }
    #endif

    etimer_reset(&timer);
  }

  PROCESS_END();
}

