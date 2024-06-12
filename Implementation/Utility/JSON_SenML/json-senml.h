#ifndef JSON_SENML_H
#define JSON_SENML_H

#define BASE_NAME_LEN 32
#define MAX_STRING_LEN 50


typedef union {
    double v;       // Numeric value
    bool bv;        // Boolean value
    char *sv;       // String value
} senml_value_t;

typedef enum {
    SENML_TYPE_V,
    SENML_TYPE_BV,
    SENML_TYPE_SV
} senml_value_type_t;

typedef struct {
    char *name;
    char *unit;
    senml_value_t value;
    senml_value_type_t type;
} senml_measurement_t;

typedef struct {
    char *base_name;
    int base_time;
    int version;
    senml_measurement_t *measurements;
    int num_measurements;
} senml_payload_t;

int create_senml_payload(char *buffer, uint16_t buffer_size, senml_payload_t *payload);
void get_mac_address(char *mac_str);
int parse_senml_payload(char *buffer, uint16_t buffer_size, senml_payload_t *payload);

#endif  // JSON_SENML_H