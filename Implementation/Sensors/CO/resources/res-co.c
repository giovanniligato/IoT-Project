#include "contiki.h"
#include "coap-engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "json-senml.h"
#include "sys/log.h"
#include "random-number-generator.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define MIN_CO_LEVEL 0.00117
#define MAX_CO_LEVEL 0.01442
#define MAX_PERCENTAGE_VARIATION 0.05

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_co,
               "title=\"VoltVault: \";rt=\"senml+json\";if=\"sensor\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);


static double co_level = -1.0;
bool hvac_status = false;

static void
res_event_handler(void)
{
    // New CO level measurement
    if(co_level < 0) 
        co_level = init_random_number(MIN_CO_LEVEL, MAX_CO_LEVEL);
    else
        co_level = generate_random_number(MIN_CO_LEVEL, MAX_CO_LEVEL, co_level, MAX_PERCENTAGE_VARIATION, hvac_status);
    
    LOG_DBG("New CO level: %f\n", co_level);
    
    // Notify all the observers
    coap_notify_observers(&res_co);
}


static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    
    static senml_measurement_t measurements[1];
    measurements[0].name = "co";
    measurements[0].type = SENML_TYPE_V;
    measurements[0].value.v = co_level;
    measurements[0].unit = "ppm";
    static char base_name[BASE_NAME_LEN];
    get_mac_address(base_name);

    static senml_payload_t payload = {
        .base_name = base_name,
        .base_time = 0,
        .version = 1,
        .measurements = measurements,
        .num_measurements = 1
    };

    int length = create_senml_payload((char *)buffer, preferred_size, &payload);

    if (length < 0) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
        LOG_ERR("[CO] Error in creating SenML payload\n");
    } else {
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_payload(response, buffer, length);
        
        // Printing the payload for debugging purposes
        LOG_DBG("[CO] Sending the payload: %s\n", buffer);
    }
}

