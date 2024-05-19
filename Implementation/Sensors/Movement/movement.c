#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/button-hal.h"
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

extern coap_resource_t res_movement;

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

  coap_activate_resource(&res_movement, "movement");

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
