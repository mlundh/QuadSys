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

/**
 * @file state_estimator.h
 *
 * StateEstimator is a core module responsible of gathering data
 * and analyzing the data to get a estimate of the current
 * stat (physical state) of the vehicle.
 *
 * Depending on the requested estimation type, different
 * states might be available for the control system to use,
 * examine the confidence field of the data to gain information
 * on what states are availible.
 */

#include "stdint.h"
#include "Utilities/inc/common_types.h"

typedef struct StateEst StateEst_t;


/*estimation methods supported.*/
typedef enum etstimation_types
{
  raw_data_rate = 1,
  kalman_filtered = 2,
  complementary_filter = 3,
  type_not_availible = 4
} estimation_types_t;

StateEst_t *StateEst_Create(CtrlModeHandler_t* CtrlModeHandler);

/**
 * Initialize the state estimator. This causes all connected and configured
 * sensors to be initialized.
 *
 * @return 0 if success else error code.*/
uint8_t StateEst_init(StateEst_t *obj);

/**
 * Gets the new state data and copies it into state.
 *
 * get_state() might take some time to complete if multiple
 * sensors has to be read.
 *
 * @param state_vector The state vector.
 * @return 0 for Success or failure code.
 * */
uint8_t StateEst_getState(StateEst_t *obj, state_data_t *state_vector);


#endif /* STATE_ESTIMATOR_H_ */
