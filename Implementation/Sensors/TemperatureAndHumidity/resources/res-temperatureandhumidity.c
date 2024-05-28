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

#define MIN_TEMPERATURE_LEVEL 0.0
#define MAX_TEMPERATURE_LEVEL 30.6
#define MAX_PERCENTAGE_VARIATION_TEMPERATURE 0.05

#define MIN_HUMIDITY_LEVEL 1.1
#define MAX_HUMIDITY_LEVEL 99.9
#define MAX_PERCENTAGE_VARIATION_HUMIDITY 0.05



static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_temperatureandhumidity,
               "title=\"VoltVault: \";rt=\"senml+json\";if=\"sensor\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);


static double temperature_level = -1.0;
static double humidity_level = -1.0;
bool hvac_status = false;

static void
res_event_handler(void)
{
    // New Measurement of temperature and humidity
    if(temperature_level < 0 && humidity_level < 0) {
        temperature_level = init_random_number(MIN_TEMPERATURE_LEVEL, MAX_TEMPERATURE_LEVEL);
        humidity_level = init_random_number(MIN_HUMIDITY_LEVEL, MAX_HUMIDITY_LEVEL);
    }
    else{
        temperature_level = generate_random_number(MIN_TEMPERATURE_LEVEL, MAX_TEMPERATURE_LEVEL, temperature_level, MAX_PERCENTAGE_VARIATION_TEMPERATURE, hvac_status);
        humidity_level = generate_random_number(MIN_HUMIDITY_LEVEL, MAX_HUMIDITY_LEVEL, humidity_level, MAX_PERCENTAGE_VARIATION_HUMIDITY, hvac_status);
    }
    
    LOG_DBG("New Temperature level: %f\n", temperature_level);
    LOG_DBG("New Humidity level: %f\n", humidity_level);
    
    // Notify all the observers
    coap_notify_observers(&res_temperatureandhumidity);
}


static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    
    static senml_measurement_t measurements[2];

    // Temperature Measurement
    measurements[0].name = "temperature";
    measurements[0].type = SENML_TYPE_V;
    measurements[0].value.v = temperature_level;
    measurements[0].unit = "Cel";
    
    // Humidity Measurement
    measurements[1].name = "humidity";
    measurements[1].type = SENML_TYPE_V;
    measurements[1].value.v = humidity_level;
    measurements[1].unit = "%RH";

    static char base_name[BASE_NAME_LEN];
    get_mac_address(base_name);

    static senml_payload_t payload = {
        .base_name = base_name,
        .base_time = 0,
        .version = 1,
        .measurements = measurements,
        .num_measurements = 2
    };

    int length = create_senml_payload((char *)buffer, preferred_size, &payload);

    if (length < 0) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
        LOG_ERR("[TemperatureAndHumidity] Error generating SenML payload\n");
    } else {
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_payload(response, buffer, length);
        
        // Printing the payload for debugging purposes
        LOG_DBG("[TemperatureAndHumidity] Sending the payload: %s\n", buffer);
    }
}

