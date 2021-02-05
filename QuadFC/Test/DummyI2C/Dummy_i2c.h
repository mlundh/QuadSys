/*
 * Dummy_i2c.h
 *
 * Copyright (C) 2016 Martin Lundh
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
#ifndef TEST_DUMMYI2C_DUMMY_I2C_H_
#define TEST_DUMMYI2C_DUMMY_I2C_H_
#include "QuadFC/QuadFC_I2c.h" // Get access to io types.
/**
 * Test helper to simulate a real i2c device. Just add expected input and
 * the corresponding response. The following read/write accesses to the
 * QuadFC_i2c* api will use the stored data. If the transaction is a write
 * then add a NULL pointer as the response.
 *
 * The implementation supports two i2c busses. These share the same data
 * storage for input-response sequences. Make sure that you address the
 * correct bus manually since the tester will not validate this for you.
 *
 * This implementation is NOT thread safe. Should only be used in single
 * thread environments, or only accessed from one thread.
 */


/**
 *
 * @param NrUsage         Number of times the sample should be replayed.
 * @param expectedInput   Expected data to be passed to the interface. Use
 *                        malloc to allocate the data for the struct since
 *                        the test will take ownership of the data. The module
 *                        will call free on the data once it is finished using
 *                        it.
 * @param response        The responce to the given expected input. Set this
 *                        to NULL if the expected input is a write access.
 *                        Use malloc to allocate the data for the struct since
 *                        the test will take ownership of the data.
 * @return                1 if ok, 0 otherwise.
 */
uint8_t DummyI2C_AddResponse(int NrUsage, QuadFC_I2C_t* expectedInput, QuadFC_I2C_t* response);

/**
 * Compare two QuadFC_I2C_t objects. All fields will be compared. Deep compare of
 * applicable fields.
 * @param first   First object.
 * @param second  Second object.
 * @return        1 if equal, 0 if not equal.
 */
uint8_t DummyI2C_Compare(const QuadFC_I2C_t* first, const QuadFC_I2C_t* second, uint8_t deepcmp);

/**
 * Populate target with data only from reference.
 * @param target
 * @param reference
 * @param deepcmp
  * @return
 */
uint8_t DummyI2C_Populate(const QuadFC_I2C_t* target, const QuadFC_I2C_t* reference);

uint8_t DummyI2C_ReadWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, uint8_t isRead);


#endif /* TEST_DUMMYI2C_DUMMY_I2C_H_ */
