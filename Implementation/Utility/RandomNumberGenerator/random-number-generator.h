#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

double init_random_number(double min, double max);
double generate_random_number(double min, double max, double old_value, double max_delta, bool hvac_status);

#endif  // RANDOM_NUMBER_GENERATOR_H