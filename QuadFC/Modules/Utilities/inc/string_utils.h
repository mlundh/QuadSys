/*
 * prepare_output.h
 *
 * Created: 2013-07-15 17:29:29
 *  Author: Martin Lundh
 */

#ifndef PREPARE_OUTPUT_H_
#define PREPARE_OUTPUT_H_

#include "stdint.h"
#include "stddef.h"
#include <string.h>

uint8_t Fc_itoa( int32_t val, uint8_t *out_str );
int32_t Fc_atoi( uint8_t *, uint8_t );

/**
 * @brief Get the string between start char and end char.
 *
 * Helper function to get the string between start and end char.
 * If start or end exists in multiple locations, the first match is used.
 * Start has to be located before end, or the function will return 0 and
 * fail.
 *
 * example:
 *
 * start char = '<'
 * end char = '>'
 *
 * stringBuffer = foo<5>
 *
 * will copy '5' to buffer and return 1.
 *
 * @param StringBuffer      Input string buffer.
 * @param stringBufferLength  Input string buffer length.
 * @param buffer        Outout buffer.
 * @param buffer_length     Outout buffer length.
 * @param start         Start char.
 * @param end         End char.
 * @return            Pointer to first element after "end". Null if fail.
 */
uint8_t *FcString_GetStringBetween(uint8_t *StringBuffer, uint32_t stringBufferLength
    , uint8_t *buffer, uint32_t buffer_length
    , const char start, const char end);

/**
 * @brief Get string until one of the delimiters.
 *
 * Copy string from start of fromBuffer to start of toBuffer until the first
 * delimiter in delimiters is found.
 *
 * Example:
 *
 * fromBuffer = "foo<1>[22]/bar<2>[2]"
 * delimiters = "/[<"
 *
 * Then the function will copy only "foo" to toBuffer.
 *
 * @param toBuffer      Buffer to copy to.
 * @param toBufferLength  Length of buffer to copy to, used for boundary check.
 * @param fromBuffer    Buffer to search for delimiter in, and copy from.
 * @param frombufferLength  Length of from buffer, used for boundary check.
 * @param delimiters    String of delimiters to search for.
 * @param noDelimiters    Number of delimiters to search for.
 * @return          pointer to first occurence. Null if fail.
 */
uint8_t *FcString_GetStringUntil(uint8_t *toBuffer, uint32_t toBufferLength, uint8_t *fromBuffer, uint32_t frombufferLength
    , const char* delimiters, uint32_t noDelimiters);

/**
 * @brief Get the value string of the module described in moduleBuffer.
 *
 * Takes a module string in moduleBuffer and searches for the value
 * string in that buffer. The string will be copied to vauleBuffer.
 *
 * Example:
 * moduleBuffer = "foo<1>[22]"
 * Then the function will copy "22" to valueBuffer.
 *
 * @param valueBuffer     Buffer to write the value to as ascii.
 * @param valueBufferLength   Length of value buffer, used for boundary check.
 * @param moduleBuffer      Buffer containing the module string copied from.
 * @param moduleBufferLength  Length of module buffer, used for boundary check.
 * @return            Pointer to first element after end of value string. Null if fail.
 */
uint8_t *FcString_GetValueString(uint8_t *valueBuffer, uint32_t valueBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

/**
 * @brief Get value type string.
 *
 * Takes a module string in moduleBuffer and searches for the value type
 * string in that buffer. The string will be copied to vauleTypeBuffer.
 *
 * Example:
 * moduleBuffer = "foo<1>[22]"
 * Then the function will copy "1" to valueTypeBuffer.
 *
 *
 * @param valueTypeBuffer     Buffer to write the value type to as ascii.
 * @param valueTypeBufferLength   Length of value type buffer, used for boundary check.
 * @param moduleBuffer        Buffer containing the module string copied from.
 * @param moduleBufferLength    Length of module buffer, used for boundary check.
 * @return              Pointer to first element after end of value type string. Null if fail.
 */
uint8_t *FcString_GetValueTypeString(uint8_t *valueTypeBuffer, uint32_t valueTypeBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

/**
 * @brief Get module string.
 *
 * Get the module string from the path described in pathBuffer. Will not include
 * delimiters.
 *
 * @param moduleBuffer      Buffer to copy module string to.
 * @param moduleBufferLength  Length of module buffer, used for boundary check.
 * @param pathBuffer      Buffer containing the path to be handled.
 * @param PathBufferLength    Length of pathBuffer, used for boundary check.
 * @return            Pointer to first element after module string. Null if fail.
 */
uint8_t *FcString_GetModuleString(uint8_t *moduleBuffer, uint32_t moduleBufferLength, uint8_t *pathBuffer, uint32_t PathBufferLength);

/**
 * @brief Get name string from module string.
 *
 * Get the name string from the module string in moduleBuffer. Name will be placed
 * at the beginning of nameBuffer.
 *
 * @param nameBuffer      Buffer to copy the name to.
 * @param nameBufferLength    Length of name buffer, used for boundary check.
 * @param moduleBuffer      Buffer containing the module string.
 * @param moduleBufferLength  module buffer length, used for boundary check.
 * @return            Pointer to first element after name sting.
 */
uint8_t *FcString_GetNameString(uint8_t *nameBuffer, uint32_t nameBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

#endif /* PREPARE_OUTPUT_H_ */
