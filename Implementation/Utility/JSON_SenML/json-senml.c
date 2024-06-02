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

    int temp = 0;
    for (size_t i = 0; i < payload->num_measurements; ++i) {
        const char *format = NULL;

        switch (payload->measurements[i].type) {
            case SENML_TYPE_V:
                temp =  (int) (payload->measurements[i].value.v * 100000);
                format = "{\"n\":\"%s\",\"v\":%d,\"u\":\"%s\"}";
                offset += snprintf(buffer + offset, buffer_size - offset, format,
                                   payload->measurements[i].name, temp, payload->measurements[i].unit);
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

    offset += snprintf(buffer + offset, buffer_size - offset, "],\"bn\":\"%s\",\"bt\":%d,\"ver\":%d}", 
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
        printf("ERROR in parse_senml_payload: POSITION 1\n");
        return -1;
    }

    int measurements_count = 0;
    char *pos = buffer;
    char *end = buffer + buffer_size;

    int temp = 0;
    while (pos < end && *pos != '\0') {
        if (strncmp(pos, "\"bn\"", 4) == 0) {
            pos += 5;
            char *start = strchr(pos, '\"');
            if (start == NULL) { 
                printf("ERROR in parse_senml_payload: POSITION 2\n"); 
                return -1;
            }
            char *finish = strchr(start + 1, '\"');
            if (finish == NULL) {
                printf("ERROR in parse_senml_payload: POSITION 3\n"); 
                return -1;
            }
            *finish = '\0';
            strncpy(payload->base_name, start + 1, MAX_STRING_LEN - 1);
            payload->base_name[MAX_STRING_LEN - 1] = '\0'; // Null-terminated string
            pos = finish + 1;
        } else if (strncmp(pos, "\"bt\"", 4) == 0) {
            pos += 5;
            payload->base_time = atoi(pos);
            pos = strchr(pos, ',');
        } else if (strncmp(pos, "\"ver\"", 5) == 0) {
            pos += 6;
            payload->version = atoi(pos);
            while (*pos != '\0' && *pos != ',' && *pos != '}') {
                pos++;
            }
        } else if (strncmp(pos, "\"e\"", 3) == 0) {
            pos += 4;
            if (*pos != '[') {
                printf("ERROR in parse_senml_payload: POSITION 4\n"); 
                return -1;
            }
            pos++;
            while (*pos != ']' && pos < end) {
                printf("DEBUG in parse_senml_payload: measurements_count: %d, payload->num_measurements: %d\n", measurements_count, payload->num_measurements);
                if (measurements_count >= payload->num_measurements) {
                    printf("ERROR in parse_senml_payload: POSITION 5\n"); 
                    return -1;
                }
                senml_measurement_t *measurement = &payload->measurements[measurements_count];

                if (strncmp(pos, "{\"n\"", 4) == 0) {
                    pos += 5;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 6\n"); 
                        return -1;
                    }
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 7\n"); 
                        return -1;
                    }
                    *finish = '\0';
                    strncpy(measurement->name, start + 1, MAX_STRING_LEN - 1);
                    measurement->name[MAX_STRING_LEN - 1] = '\0'; // Null-terminated string
                    pos = finish + 2;
                }

                if (strncmp(pos, "\"u\"", 3) == 0) {
                    pos += 4;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 8\n"); 
                        return -1;
                    }
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 9\n"); 
                        return -1;
                    }
                    *finish = '\0';
                    strncpy(measurement->unit, start + 1, MAX_STRING_LEN - 1);
                    measurement->unit[MAX_STRING_LEN - 1] = '\0'; // Null-terminated string
                    pos = finish + 2;
                }

                if (strncmp(pos, "\"v\"", 3) == 0) {
                    pos += 4;
                    measurement->type = SENML_TYPE_V;
                    temp = atoi(pos);
                    measurement->value.v = ((double) temp) / 100000.0;
                    // measurement->value.v = atof(pos);
                    pos = strchr(pos, ',');
                } else if (strncmp(pos, "\"bv\"", 4) == 0) {
                    pos += 5;
                    measurement->type = SENML_TYPE_BV;
                    measurement->value.bv = (strncmp(pos, "true", 4) == 0);
                    while (*pos != '\0' && *pos != ',' && *pos != '}') {
                        pos++;
                    }
                } else if (strncmp(pos, "\"sv\"", 4) == 0) {
                    pos += 5;
                    char *start = strchr(pos, '\"');
                    if (start == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 10\n"); 
                        return -1;
                    }
                    char *finish = strchr(start + 1, '\"');
                    if (finish == NULL) {
                        printf("ERROR in parse_senml_payload: POSITION 11\n"); 
                        return -1;
                    }
                    *finish = '\0';
                    strncpy(measurement->value.sv, start + 1, MAX_STRING_LEN - 1);
                    measurement->value.sv[MAX_STRING_LEN - 1] = '\0'; // Null-terminated string
                    pos = finish + 2;
                }

                measurements_count++;
                pos = strchr(pos, '}');
                if (pos == NULL) {
                    printf("ERROR in parse_senml_payload: POSITION 12\n"); 
                    return -1;
                }
                pos++;
                if (*pos == ',') pos++;
            }
        }
        pos++;
    }

    return 0;
}
