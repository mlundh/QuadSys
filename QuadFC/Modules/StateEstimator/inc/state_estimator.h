/*
 * state_estimator.h
 *
 * Copyright (C) 2014 martin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef STATE_ESTIMATOR_H_
#define STATE_ESTIMATOR_H_

#include "stdint.h"
#include "common_types.h"


/*estimation methods supported.*/
typedef enum etstimation_types
{
  raw_data = 1,
  kalman_filtered = 2,
  complementary_filter = 3,
} estimation_types_t;

/**
 * Initialize the state estimator. This causes all connected and configured
 * sensors to be initialized.
 *
 * @return 0 if success else error code.*/
uint8_t initialize_state_estimator(estimation_types_t type);


/**
 * Check if a named state variable is available with the current setup.
 *
 * @param *state_vector the state vector.
 * @param state: named state.
 * @return state availability, 0 if unavailable, !0 otherwise.  */
uint8_t is_state_available(state_data_t *state_vector, state_names_t state);

/**
 * Get the current state confidence, updated with each update of the state vector.
 *
 * @param state_vector the state vector.
 * @param state named state.
 * @return state confidence.
 * */
uint8_t get_state_confidence(state_data_t *state_vector, state_names_t state);

/**
 * Gets the new state data and copies it into state.
 *
 * get_state() might take some time to complete if multiple
 * sensors has to be read.
 *
 * @param state_vector The state vector.
 * @return 0 for Success or failure code.
 * */
uint8_t get_state(state_data_t *state_vector);

/**
 * Set the desired estimation type. The different estimation types
 * will provide a different number of estimated states and different
 * confidence.
 *
 * @param est requested estimation type
 */
void config_set_estimation_type(estimation_types_t est);

/**
 * Get the current estimation type described in the estimation
 * type enumeration.
 * @return current estimation type.
 */
estimation_types_t get_estimation_type();


/*private section, to be moved to .c file*/

void set_state_confidence(state_data_t state_vector, state_names_t state);


#endif /* STATE_ESTIMATOR_H_ */
