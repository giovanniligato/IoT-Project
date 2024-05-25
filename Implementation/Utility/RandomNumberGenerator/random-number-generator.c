#include <stdbool.h>
#include "random.h"

double init_random_number(double min, double max){
    
    // Generating a random number between 0 and 1
    double random_value = random_rand() / RANDOM_RAND_MAX;

    // Scale and shift to the desired range [min_value, max_value]
    return min + (max - min) * random_value;
}

double generate_random_number(double min, double max, double old_value, double max_delta, bool hvac_status){
    double new_value;
    double mean = (min + max) / 2;
    double max_variation = max_delta * mean;      

    double variation = (random_rand() / RANDOM_RAND_MAX) * max_variation;   // value between 0 and max_variation
    

    if(hvac_status){
        // HVAC is on
        new_value = old_value - variation;                              // value between min_variation and new_value+variation
        new_value = new_value < min ? min : new_value;
    }
    else{
        // HVAC is off
        new_value = old_value + 2 * variation - max_variation;          // value between new_value-variation and max_variation
        new_value = new_value > max ? max : new_value;
        new_value = new_value < min ? min : new_value;
    }

    return new_value;
}