#include <nrfx.h>
#include <stdio.h>

static void get_mac_address(char *mac_str) {
    uint8_t mac[6];
    mac[0] = (NRF_FICR->DEVICEADDR[1] >> 8) & 0xFF;
    mac[1] = (NRF_FICR->DEVICEADDR[1] >> 0) & 0xFF;
    mac[2] = (NRF_FICR->DEVICEADDR[0] >> 24) & 0xFF;
    mac[3] = (NRF_FICR->DEVICEADDR[0] >> 16) & 0xFF;
    mac[4] = (NRF_FICR->DEVICEADDR[0] >> 8) & 0xFF;
    mac[5] = (NRF_FICR->DEVICEADDR[0] >> 0) & 0xFF;
    
    snprintf(mac_str, 24, "urn:dev:mac:%02X%02X%02X%02X%02X%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static int create_senml_payload(char *buffer, size_t buffer_size, const char *base_unit, int base_version, const char *name, bool value)
{
    if (buffer == NULL || base_name == NULL || base_unit == NULL || name == NULL) {
        return -1;
    }

    char base_name[24];
    get_mac_address(base_name);

    double base_time = (double)get_time_counter(); // Use the time counter from movement.c

    int length = snprintf(buffer, buffer_size,
                          "[{\"bn\":\"%s\",\"bt\":%.0f,\"bu\":\"%s\",\"bver\":%d,\"n\":\"%s\",\"v\":%d}]",
                          base_name, base_time, base_unit, base_version, name, value);

    if (length < 0 || length >= buffer_size) {
        return -1; // Error or buffer overflow
    }

    return length;
}
