#include "contiki.h"
#include "coap-engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "json-senml.h"
#include "sys/log.h"
#include "machine_learning.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_hvac,
               "title=\"VoltVault: \";rt=\"senml+json\";if=\"actuator\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static bool hvac_status = false;

extern double current_temperature;
extern double current_humidity;
extern double current_co;

static void
res_event_handler(void)
{
    
    // Prepare input data
    float input_data[3] = {(float)current_temperature, (float)current_humidity, (float)current_co};

    // Predict the status of the HVAC using 
    // the ML model. The values returned by
    // the ML model are:
    // 1: Habitable, 
    // 0: Not habitable
    //
    // The HVAC needs to be turned ON if the
    // vault is NOT habitable. So hvac_status
    // is the negation of the predicted value.
    hvac_status = machine_learning_predict(input_data, 3) == 0;
    LOG_DBG("[HVAC] Predicted HVAC status: %d\n", hvac_status);

    // Notify all the observers
    coap_notify_observers(&res_hvac);
}


static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

    static senml_measurement_t measurements[1];
    measurements[0].name = "hvac";
    measurements[0].type = SENML_TYPE_BV;
    measurements[0].value.bv = hvac_status;
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
        LOG_ERR("[HVAC] Error in creating SenML payload\n");
    } else {
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_payload(response, buffer, length);
        
        // Printing the payload for debugging purposes
        LOG_DBG("[HVAC] Sending the payload: %s\n", buffer);
    }
}

