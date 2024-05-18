#include "contiki.h"
#include "coap-engine.h"
#include <stdio.h>
#include <stdlib.h>
#include "json-senml.h"

extern uint32_t get_time_counter(void);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_movement,
               "title=\"VoltVault: \";rt=\"senml+json\";if=\"sensor\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static bool vault_activated = false;

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
    const char *base_unit = "boolean";
    int base_version = 1;
    const char *name = "vault_activated";

    int length = create_senml_payload((char *)buffer, preferred_size, base_unit, base_version, name, vault_activated);

    if (length < 0) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    } else {
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_payload(response, buffer, length);
    }
}

