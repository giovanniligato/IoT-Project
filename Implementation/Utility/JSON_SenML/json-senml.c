#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
// #include "debug_sleep.h"
// #include "sys/clock.h"

#ifndef COOJA
#include <nrfx.h>
#endif

#include <stdint.h>
#include "json-senml.h"

#define MAX_MEASUREMENTS 2 // Definisci il numero massimo di misurazioni che il buffer può contenere

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

    // Salva il locale corrente
    char *current_locale = setlocale(LC_NUMERIC, NULL);

    // Imposta il locale su "C" per usare il punto come separatore decimale
    setlocale(LC_NUMERIC, "C");

    size_t offset = 0;
    offset += snprintf(buffer + offset, buffer_size - offset, "{\"e\":[");

    for (size_t i = 0; i < payload->num_measurements; ++i) {
        const char *format = NULL;

        switch (payload->measurements[i].type) {
            case SENML_TYPE_V:
                format = "{\"n\":\"%s\",\"v\":%.5f,\"u\":\"%s\"}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.v, payload->measurements[i].unit);
                break;
            case SENML_TYPE_BV:
                format = "{\"n\":\"%s\",\"bv\":%s}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.bv ? "true" : "false");
                break;
            case SENML_TYPE_SV:
                format = "{\"n\":\"%s\",\"sv\":\"%s\",\"u\":\"%s\"}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, payload->measurements[i].value.sv, payload->measurements[i].unit);
                break;
            default:
                // Ripristina il locale originale
                setlocale(LC_NUMERIC, current_locale);
                return -1;
        }

        if (i < payload->num_measurements - 1) {
            offset += snprintf(buffer + offset, buffer_size - offset, ",");
        }
    }

    offset += snprintf(buffer + offset, buffer_size - offset, "],\"bn\":\"%s\",\"bt\":%.0f,\"ver\":%d}", 
                       payload->base_name, payload->base_time, payload->version);

    // Ripristina il locale originale
    setlocale(LC_NUMERIC, current_locale);
    
    if (offset >= buffer_size) {
        return -1; // Error or buffer overflow
    }

    return offset;
}

int parse_senml_payload(char *buffer, uint16_t buffer_size, senml_payload_t *payload) {
    if (buffer == NULL || payload == NULL) {
        return -1;
    }

    // Inizializza il payload
    payload->measurements = (senml_measurement_t *)malloc(MAX_MEASUREMENTS * sizeof(senml_measurement_t));
    if (payload->measurements == NULL) {
        return -1;
    }

    payload->num_measurements = 0;
    char *pos = buffer;
    char *end = buffer + buffer_size;

    while (pos < end && *pos != '\0') {
        // Cerca il campo "bn"
        if (strncmp(pos, "\"bn\"", 4) == 0) {
            pos += 5;
            char *start = strchr(pos, '\"');
            if (start == NULL) return -1;
            char *finish = strchr(start + 1, '\"');
            if (finish == NULL) return -1;
            *finish = '\0';
            payload->base_name = strdup(start + 1);
            pos = finish + 1;
        }
        // Cerca il campo "bt"
        else if (strncmp(pos, "\"bt\"", 4) == 0) {
            pos += 5;
            payload->base_time = atof(pos);
            pos = strchr(pos, ',');
        }
        // Cerca il campo "ver"
        else if (strncmp(pos, "\"ver\"", 5) == 0) {
            pos += 6;
            payload->version = atoi(pos);
            while (*pos != '\0' && *pos != ',' && *pos != '}') {
                pos++;
            }
        }
        // Cerca il campo "e"
        else if (strncmp(pos, "\"e\"", 3) == 0) {
            pos += 4;
            if (*pos != '[') return -1;
            pos++;
            while (*pos != ']' && pos < end) {

                if (payload->num_measurements >= MAX_MEASUREMENTS) return -1;
                
                senml_measurement_t *measurement = &payload->measurements[payload->num_measurements];
                memset(measurement, 0, sizeof(senml_measurement_t));
                
                // Cerca il campo "n"
                if (strncmp(pos, "{\"n\"", 4) == 0) {
                    pos += 5;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) return -1;
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) return -1;
                    *finish = '\0';
                    measurement->name = strdup(start + 1);
                    pos = finish + 2;
                }

                // Cerca il campo "u"
                if (strncmp(pos, "\"u\"", 3) == 0) {
                    pos += 4;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) return -1;
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) return -1;
                    *finish = '\0';
                    measurement->unit = strdup(start + 1);
                    pos = finish + 2;
                }
                
                // Cerca il campo "v"
                if (strncmp(pos, "\"v\"", 3) == 0) {
                    pos += 4;
                    measurement->type = SENML_TYPE_V;
                    measurement->value.v = atof(pos);
                    pos = strchr(pos, ',');
                }
                // Cerca il campo "bv"
                else if (strncmp(pos, "\"bv\"", 4) == 0) {
                    pos += 5;
                    measurement->type = SENML_TYPE_BV;
                    measurement->value.bv = (strncmp(pos, "true", 4) == 0);
                    // pos = strchr(pos, ',');
                    while (*pos != '\0' && *pos != ',' && *pos != '}') {
                        pos++;
                    }
                }

                // Cerca il campo "sv"
                else if (strncmp(pos, "\"sv\"", 4) == 0) {
                    pos += 5;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) return -1;
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) return -1;
                    *finish = '\0';
                    measurement->type = SENML_TYPE_SV;
                    measurement->value.sv = strdup(start + 1);
                    pos = finish + 2;
                }
                
                payload->num_measurements++;
                
                // Trova la fine di questa misurazione
                pos = strchr(pos, '}');
                if (pos == NULL) return -1;
                pos++;
                if (*pos == ',') pos++;
            }
        }

        pos++;
    }

    return 0;
}
