#include "contiki.h"
#include "coap-engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "json-senml.h"
#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_movement,
               "title=\"VoltVault: \";rt=\"senml+json\";if=\"sensor\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

// static bool vault_activated = false;
static double co_level = 0.0;
#define MIN_CO_LEVEL 0.00117
#define MAX_CO_LEVEL 0.01442
#define MAX_PERCENTAGE_VARIATION 5


// extract a random number between min and max
// Creare funzione completa randomica che non restituisce numeri
// a caso, ma è intelligente e fa in modo che il valore randomico
// sia tipo tra +-5% del valore precedente e in più se tipo un booleano
// è settato (HVAC acceso) allora restituisce solo valori randomici
// che sono -5% del valore precedente. 

// Fare funzione di parsing per JSON perché per adesso abbiamo solo funzione che crea il JSON



minimo + (rand() % (maximo+1 - minimo))

static void
res_event_handler(void)
{
    vault_activated = !vault_activated;
    // Notify all the observers
    coap_notify_observers(&res_movement);
}


static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    senml_measurement_t measurements[1];
    measurements[0].name = "movement";
    measurements[0].type = SENML_TYPE_BV;
    measurements[0].value.bv = vault_activated;
    char base_name[BASE_NAME_LEN];
    get_mac_address(base_name);

    senml_payload_t payload = {
        .base_name = base_name,
        .base_time = 0,
        .version = 1,
        .measurements = measurements,
        .num_measurements = 1
    };

    int length = create_senml_payload((char *)buffer, preferred_size, &payload);

    if (length < 0) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    } else {
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_payload(response, buffer, length);
        
        // Printing the payload for debugging purposes
        LOG_DBG("Payload: %s\n", buffer);
    }
}

