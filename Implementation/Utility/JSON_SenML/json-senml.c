#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef COOJA
#include <nrfx.h>
#endif

#include <stdint.h>
#include "json-senml.h"

void get_mac_address(char *mac_str) {
    
    uint8_t mac[6];
    #ifndef COOJA
    mac[0] = (NRF_FICR->DEVICEADDR[1] >> 8) & 0xFF;
    mac[1] = (NRF_FICR->DEVICEADDR[1] >> 0) & 0xFF;
    mac[2] = (NRF_FICR->DEVICEADDR[0] >> 24) & 0xFF;
    mac[3] = (NRF_FICR->DEVICEADDR[0] >> 16) & 0xFF;
    mac[4] = (NRF_FICR->DEVICEADDR[0] >> 8) & 0xFF;
    mac[5] = (NRF_FICR->DEVICEADDR[0] >> 0) & 0xFF;
    #else
    // Genera un indirizzo MAC fittizio per la simulazione in Cooja
    mac[0] = 0x02; // UAA (Universally Administered Address) con un bit LSB impostato a 0
    mac[1] = 0x00;
    mac[2] = 0x00;
    mac[3] = 0x00;
    mac[4] = 0x00;
    mac[5] = 0x01;
    #endif
    
    snprintf(mac_str, BASE_NAME_LEN, "urn:dev:mac:%02X%02X%02X%02X%02X%02X:", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int create_senml_payload(char *buffer, uint16_t buffer_size, senml_payload_t *payload)
{
    if (buffer == NULL || payload == NULL || payload->measurements == NULL || payload->num_measurements == 0) {
        return -1;
    }

    size_t offset = 0;
    offset += snprintf(buffer + offset, buffer_size - offset, "{\"e\":[");

    for (size_t i = 0; i < payload->num_measurements; ++i) {
        const char *format = NULL;

        switch (payload->measurements[i].type) {
            case SENML_TYPE_V:
                format = "{\"n\":\"%s\",\"v\":%.2f,\"u\":\"%s\"}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.v, payload->measurements[i].unit);
                break;
            case SENML_TYPE_BV:
                format = "{\"n\":\"%s\",\"bv\":%s}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.bv ? "true" : "false");
                printf("BV: %s\n", payload->measurements[i].value.bv ? "true" : "false");
                break;
            case SENML_TYPE_SV:
                format = "{\"n\":\"%s\",\"sv\":\"%s\",\"u\":\"%s\"}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.sv, payload->measurements[i].unit);
                break;
            default:
                return -1;
        }

        if (i < payload->num_measurements - 1) {
            offset += snprintf(buffer + offset, buffer_size - offset, ",");
        }
    }

    offset += snprintf(buffer + offset, buffer_size - offset, "],\"bn\":\"%s\",\"bt\":%.0f,\"ver\":%d}", 
                       payload->base_name, payload->base_time, payload->version);

    if (offset >= buffer_size) {
        return -1; // Error or buffer overflow
    }

    return offset;
}