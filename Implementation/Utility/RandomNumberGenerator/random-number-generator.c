#include <stdbool.h>
#include "random.h"

/**
 * Generates a random number within a specified range.
 *
 * @param min The minimum value of the desired range.
 * @param max The maximum value of the desired range.
 * @return A random number between min and max.
 */
double init_random_number(double min, double max){
    
    // Generating a random number between 0 and 1
    double random_value =  (double) random_rand() / RANDOM_RAND_MAX;

    // Scale and shift the random value to the desired range [min, max]
    return min + (max - min) * random_value;
}

/**
 * Generates a new random number based on an old value and additional parameters.
 *
 * @param min The minimum value of the desired range.
 * @param max The maximum value of the desired range.
 * @param old_value The previous value to base the new random value on.
 * @param max_delta The maximum allowed variation from the mean.
 * @param hvac_status The status of the HVAC system (true if on, false if off).
 * @return A new random number within the adjusted range.
 */
double generate_random_number(double min, double max, double old_value, double max_delta, bool hvac_status){
    double new_value;
    double mean = (min + max) / 2;                                              // Calculate the mean of the range
    double max_variation = max_delta * mean;                                    // Determine the maximum variation based on max_delta and mean

    // Generate a random variation value between 0 and max_variation
    double variation = ((double) random_rand() / RANDOM_RAND_MAX) * max_variation;
    

    if(hvac_status){
        // If HVAC is on, decrease the old value by the variation
        new_value = old_value - variation;
        // Ensure the new value does not go below the minimum
        new_value = new_value < min ? min : new_value;
    }
    else{
        // If HVAC is off, increase/decrease the old value with some offset based on variation
        new_value = old_value + 2 * variation - max_variation;          
        // Ensure the new value does not exceed the maximum or go below the minimum
        new_value = new_value > max ? max : new_value;
        new_value = new_value < min ? min : new_value;
    }

    return new_value;
}
