/*
 * parameters_internal.h
 *
 * Copyright (C) 2019 Martin Lundh
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


#ifndef COMPONENTS_PARAMETERS_INC_PARAMETERS_INTERNAL_H_
#define COMPONENTS_PARAMETERS_INC_PARAMETERS_INTERNAL_H_

#include "parameters.h"

/**
 * @brief Add child to current.
 *
 * Register a child with the current node. The current node has to have
 * at least one space left in its child array.
 *
 * @param current Node to register child with.
 * @param child   Child to register.
 */
void Param_SetChild(param_obj_t *current, param_obj_t *child);

/**
 * @brief Set parent of current node.
 *
 * Set the parent of current node. All nodes should have a parent unless it is
 * a root node.
 * @param current   The current node.
 * @param parent    Parent node of current.
 */
void Param_SetParent(param_obj_t *current, param_obj_t *parent);

/**
 * @brief Get parent node.
 *
 * Gets the parent node of current. Returns NULL if no parent is registerd.
 *
 * @param current   Current node.
 * @return          Pointer to parent, or null if no parent exists.
 */
param_obj_t *Param_GetParent(param_obj_t *current);

/**
 * @brief Get child nr index of current.
 *
 * Get child from current node. If index is larger than the registered number
 * of children, then the function will return NULL.
 *
 * @param current   Current node.
 * @param index     Index of child to return.
 * @return          Pointer to child. NULL if no child with that index exist.
 */
param_obj_t *Param_GetChild(param_obj_t *current, uint8_t index);

/**
 * @brief Append divider "/"
 *
 * Append the divider "/" to the buffer after doing a boundary check.
 *
 * @param buffer        Buffer to append to.
 * @param buffer_length Buffer length, used for boundary check.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Param_AppendDivider( uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Append parent notation "../"
 *
 * Appends the parent notation to buffer after doing a boundary check. A trailing divider
 * will also be appended.
 *
 * @param buffer            Buffer to append to.
 * @param buffer_length     Buffer length, used for boundary check.
 * @return                  0 if fail, 1 otherwise.
 */
uint8_t Param_AppendParentNotation( uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Get value from current node.
 *
 * Gets the value of the current node, represented as ascii. Will write to the
 * start of buffer. Null terminates the string.
 *
 * @param current       Current node.
 * @param buffer        Buffer to write the value to. Writes to the start of buffer.
 * @param buffer_length Length of buffer, used for boundary check.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Param_GetNodeValue(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Set the node value of current.
 *
 * Set the value of the current node. The value is represented as ascii in buffer.
 *
 * @param current   Current node.
 * @param buffer    Buffer containing the value.
 * @return          0 if fail, 1 otherwise.
 */
uint8_t Param_SettNodeValue(param_obj_t *current, uint8_t *buffer);

/**
 * @brief Append full node string to buffer.
 *
 * Appends current node represented as a string to the end of the null terminated buffer.
 *
 * Will fail if the buffer is not long enough.
 *
 * @param current       Current node.
 * @param buffer        Buffer to append to.
 * @param buffer_length Total length of buffer, used for boundary check.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Param_AppendNodeString(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Append path to current node.
 *
 * Append path from root to and including current node to buffer.
 *
 * Will fail if buffer is not long enough.
 *
 * @param current       Current node.
 * @param buffer        Buffer to append to.
 * @param buffer_length Total length of buffer, used for boundary check
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Param_AppendPathToHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Compare value type of current with value type in buffer.
 *
 * Compares value type of current with the value type in valueTypeBuffer.
 *
 * @param current           Current node.
 * @param valueTypeBuffer   Buffer containing the value type.
 * @return                  0 if fail, 1 otherwise.
 */
uint8_t Param_CompareValueType(param_obj_t *current, uint8_t *valueTypeBuffer);

/**
 * @brief Update current node using the module string in moduleBuffer.
 *
 * Update the current node only. Use the module string in moduleBuffer.
 * If module buffer contains a type, then check the type. If module
 * buffer contains a value then try to update using that value. Validate
 * that the name is correct.
 *
 * @param current               Current node.
 * @param moduleBuffer          Buffer containing the module string used to update with.
 * @param moduleBuffer_length   Length of module buffer, used for boundary check.
 * @return                      0 if fail, 1 if otherwise.
 */
uint8_t Param_UpdateCurrent(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBuffer_length);

/**
 * @brief Find the module in moduleBuffer.
 *
 * Find the next node, described in moduleBuffer. Will find children or
 * parent of current node.
 *
 * @param current               Current node.
 * @param moduleBuffer          Buffer containing the node to find.
 * @param moduleBufferLength    Length of buffer, used for boundary check.
 * @return                      Pointer to found object. Null if fail.
 */
param_obj_t * Param_FindNext(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBufferLength);


#endif /* COMPONENTS_PARAMETERS_INC_PARAMETERS_INTERNAL_H_ */
