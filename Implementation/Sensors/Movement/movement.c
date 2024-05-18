#include "contiki.h"
#include "gpio-hal-arch.h"
#include "nrfx_gpiote.h"
#include "nrfx_rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include "sys/etimer.h"
#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define PIR_SENSOR_PORT 0  // Port 0
#define PIR_SENSOR_PIN  29 // Pin 29



extern coap_resource_t res_movement;


static uint32_t time_counter = 0; // Contatore di secondi

PROCESS(pir_motion_sensor_process, "PIR Motion Sensor Process");
AUTOSTART_PROCESSES(&pir_motion_sensor_process);

// RTC configuration
const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(2); // Use RTC instance 2

void rtc_handler(nrfx_rtc_int_type_t int_type) {
    if (int_type == NRFX_RTC_INT_TICK) {
        time_counter++;
    }
}

void rtc_config(void) {
    nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
    config.prescaler = RTC_FREQ_TO_PRESCALER(1); // Configure for 1 Hz ticks

    nrfx_rtc_init(&rtc, &config, rtc_handler);
    nrfx_rtc_tick_enable(&rtc, true);
    nrfx_rtc_enable(&rtc);
}

void initialize_time_counter(uint32_t initial_timestamp) {
    time_counter = initial_timestamp;
}

uint32_t get_time_counter(void) {
    return time_counter;
}


PROCESS_THREAD(pir_motion_sensor_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  // Inizializza il driver GPIOTE se non è già inizializzato
  if(!nrfx_gpiote_is_init()) {
    nrfx_gpiote_init();
  }

  // Configura il pin del sensore PIR come input
  gpio_hal_arch_pin_set_input(PIR_SENSOR_PORT, PIR_SENSOR_PIN);

  // Inizializza l'RTC
  rtc_config();

  // Inizializza il contatore di tempo con un valore di timestamp da un server esterno
  uint32_t initial_timestamp = 1684500000; // Esempio di timestamp iniziale
  // Chiamata a una funzione per inizializzare il contatore di tempo dal server al momento della registrazione
  initialize_time_counter(initial_timestamp);

  coap_activate_resource(&res_movement, "movement");

  // Imposta un timer per verificare lo stato del sensore ogni 500 ms
  etimer_set(&timer, CLOCK_SECOND / 2);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Legge lo stato del pin del sensore PIR
    if(nrf_gpio_pin_read(NRF_GPIO_PIN_MAP(PIR_SENSOR_PORT, PIR_SENSOR_PIN))) {
      // Chiamata a una funzione per notificare il movimento (trigger)
      res_movement.trigger();
    }

    etimer_reset(&timer);
  }

  PROCESS_END();
}


