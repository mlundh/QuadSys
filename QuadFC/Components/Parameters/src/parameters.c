/**
 * @file parameters.c
 *
 * Implementation of all functions related to parameters. Declaration of
 * private helper functions.
 *
 *  Created on: Jul 24, 2015
 *      Author: martin
 */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "Parameters/inc/parameters_internal.h"
#include "Utilities/inc/string_utils.h"


void Param_SetChild(param_obj_t *current, param_obj_t *child)
{
    if(!current || !child || !(current->registered_children < current->num_variables))
    {
        configASSERT(0);
    }
    current->children[current->registered_children++] = child;
}

param_obj_t *Param_CreateObj(uint8_t num_children, Log_variable_type_t type,
        Log_variable_access_t access, void *value, const char *obj_name,
        param_obj_t *parent)
{

    if((variable_type_NoType != type) && !value)
    {
        return NULL;
    }
    param_obj_t *log_obj = pvPortMalloc( sizeof(param_obj_t) );
    if(!log_obj)
    {
        return NULL;
    }
    log_obj->num_variables = num_children;
    log_obj->type = type;
    log_obj->access = access;
    log_obj->value = value;
    log_obj->group_name = pvPortMalloc( sizeof( unsigned char ) * MAX_PARAM_NAME_LENGTH );
    log_obj->parent = parent;
    log_obj->registered_children = 0;
    log_obj->callback = NULL;
    log_obj->cbData = NULL;
    if(num_children)
    {
        log_obj->children = pvPortMalloc(sizeof( param_obj_t *) * num_children);
        if(!log_obj->children)
        {
            return NULL;
        }
    }
    else
    {
        log_obj->children = NULL;
    }
    if(!log_obj  || ! log_obj->group_name)
    {
        return NULL;
    }

    //Set name.
    strncpy( (char *) log_obj->group_name , obj_name, (unsigned short) MAX_PARAM_NAME_LENGTH - 1);
    log_obj->group_name[MAX_PARAM_NAME_LENGTH - 1] = '\0';

    if(parent)
    {
        Param_SetChild(parent, log_obj);
    }
    return log_obj;
}

void Param_DeleteObj(param_obj_t* obj)
{
    if(!obj)
    {
        return;
    }
    vPortFree(obj->group_name);
    if(obj->children)
    {
        vPortFree(obj->children);
    }
    vPortFree(obj);

}

void Param_SetCB(param_obj_t* obj, paramCB cb, void* cbData)
{
    if(!obj || !cb)
    {
        return;
    }
    obj->callback = cb;
    obj->cbData = cbData;
}


void Param_SetParent(param_obj_t *current, param_obj_t *parent)
{
    if(!current || !parent)
    {
        return; //TODO error
    }
    current->parent = parent;
}

param_obj_t *Param_GetParent(param_obj_t *current)
{
    return current->parent;
}

param_obj_t *Param_GetChild(param_obj_t *current, uint8_t index)
{
    if(!current || !(current->registered_children < index))
    {
        return NULL;
    }
    return current->children[index];
}

uint8_t Param_AppendDivider( uint8_t *buffer, uint32_t buffer_length)
{
    if(!buffer)
    {
        return 0;
    }
    uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);

    //ensure that the name will fit in current buffer.
    if((buf_index + 2) > buffer_length) // one char + null
    {
        return 0;
    }
    strcat( (char *) buffer, (const char *) "/");
    return 1;
}

uint8_t Param_AppendParentNotation( uint8_t *buffer, uint32_t buffer_length)
{
    if(!buffer)
    {
        return 0;
    }
    uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);

    //ensure that the name will fit in current buffer.
    if((buf_index + 4) > buffer_length) // 3 char + null
    {
        return 0;
    }
    strcat( (char *) buffer, (const char *) "../");
    return 1;
}

uint8_t Param_GetNodeValue(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length)
{
    if(!current || !buffer || (buffer_length < MAX_DIGITS_INT32))
    {
        return 0;
    }

    uint8_t result = 0;

    uint8_t pTemp[11];
    switch (current->type)
    {
    case variable_type_NoType:
        break;
    case variable_type_uint8:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%hu", (*(uint8_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_uint16:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%u", (*(uint16_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_uint32:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%lu", (*(uint32_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_int8:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%hi", (*(int8_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_int16:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%i", (*(int16_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_int32:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%li", (*(int32_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_fp_16_16:
        snprintf((char *) pTemp, MAX_DIGITS_INT32, "%li", (*(int32_t*)current->value));
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
        result = 1;
        break;
    case variable_type_last:
        break;
    default:
        break;
    }
    return result;
}

uint8_t Param_SettNodeValue(param_obj_t *current, uint8_t *buffer)
{
    if(!current || !buffer)
    {
        return 0;
    }
    // Check if we have anything to update with. For now it is ok to try to set a read only,
    // it will not be set though.
    if(!(strlen((char *)buffer) > 0) || (current->access != readWrite))
    {
        return 1; // OK to not update value if there is nothing to update with.
    }

    uint8_t result = 0;
    char *end;
    long int value = strtol((char *) buffer, &end, 10);

    switch (current->type)
    {
    case variable_type_NoType:
        break;
    case variable_type_uint8:
        if(value < UINT8_MAX)
        {
            *((uint8_t*)current->value) = (uint8_t)value;
            result = 1;
        }
        break;
    case variable_type_uint16:
        if(value < UINT16_MAX)
        {
            *((uint16_t*)current->value) = (uint16_t)value;
            result = 1;
        }
        break;
    case variable_type_uint32:
        if(value < UINT32_MAX)
        {
            *((uint32_t*)current->value) = (uint32_t)value;
            result = 1;
        }
        break;
    case variable_type_int8:
        if(value < INT8_MAX)
        {
            *((int8_t*)current->value) = (int8_t)value;
            result = 1;
        }
        break;
    case variable_type_int16:
        if(value < INT16_MAX)
        {
            *((int16_t*)current->value) = (int16_t)value;
            result = 1;
        }
        break;
    case variable_type_int32:
        if(value < INT32_MAX)
        {
            *((int32_t*)current->value) = (int32_t)value;
            result = 1;
        }
        break;
    case variable_type_fp_16_16:
        if(value < INT32_MAX)
        {
            *((int32_t*)current->value) = (int32_t)value;
            result = 1;
        }
        break;
    case variable_type_last:
        break;
    default:
        break;
    }
    // If we updated the value, then we should call the optional callback.
    if(result)
    {
        if(current->callback)
        {
            current->callback(current->cbData);
        }
    }
    return result;
}
uint8_t Param_AppendNodeString(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length)
{
    if(!buffer || !current)
    {
        return 0;
    }
    uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);

    //ensure that the name will fit in current buffer.
    if((buf_index + MAX_PARAM_NODE_LENGTH) > buffer_length)
    {
        return 0;
    }
    //append group name and update buf_index.
    strncat( (char *) buffer , (const char *) current->group_name, (unsigned short) MAX_PARAM_NAME_LENGTH);
    // append variabletype "<y>" and value "[xxx]"
    strcat( (char *) buffer, (const char *) "<");
    uint8_t pTemp[MAX_VALUE_TYPE_LENGTH];
    snprintf((char *) pTemp, MAX_VALUE_TYPE_LENGTH, "%lu",(uint32_t)current->type);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_VALUE_TYPE_LENGTH);
    strcat( (char *) buffer, (const char *) ">");
    if(variable_type_NoType != current->type)
    {
        strcat( (char *) buffer, (const char *) "[");
        if(!Param_GetNodeValue(current, buffer, buffer_length))
        {
            return 0;
        }
        strcat( (char *) buffer, (const char *) "]");
    }
    return 1;
}

uint8_t Param_AppendPathToHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length)
{
    if(!buffer || !current)
    {
        return 0;
    }

    //Get handles to all parents.
    param_obj_t * parentBuffer[MAX_DEPTH];
    param_obj_t * parent = Param_GetParent(current);
    uint32_t parentIdx = 0;
    if(!parent)
    {
        if(!Param_AppendDivider(buffer, buffer_length))
        {
            return 0;
        }
        return 1; // it is ok to not have a parent.
    }
    while(parent && (parentIdx < MAX_DEPTH))
    {

        parentBuffer[parentIdx] = parent;
        parent = Param_GetParent(parent);
        if(parent)
        {
            parentIdx++;
        }
    }

    //Ensure that the parent strings will fit in the buffer.
    uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);
    if((buf_index + ((MAX_PARAM_NODE_LENGTH + 1) * parentIdx) ) > buffer_length)
    {
        return 0;
    }

    //Print all parents to buffer in correct order.
    for(uint32_t i = 0; i <= parentIdx; i++)
    {
        if(!Param_AppendDivider(buffer, buffer_length))
        {
            return 0;
        }
        if(!Param_AppendNodeString(parentBuffer[parentIdx - i], buffer, buffer_length))
        {
            return 0;
        }

    }
    if(!Param_AppendDivider(buffer, buffer_length))
    {
        return 0;
    }
    return 1;
}


uint8_t Param_AppendDumpFromHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper)
{
    if(!buffer || !current || (helper->depth > MAX_DEPTH))
    {
        return 0;
    }
    //ensure that the name will fit in current buffer.

    uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);
    if((buf_index + MAX_PARAM_NODE_LENGTH) > buffer_length)
    {
        return 0;
    }

    // Dump self.
    if(!Param_AppendNodeString(current, buffer, buffer_length))
    {
        return 0;
    }
    if(!Param_AppendDivider( buffer, buffer_length))
    {
        return 0;
    }
    //Dump children.
    for(uint8_t i = helper->dumpStart[helper->depth++]; i < current->registered_children; i++)
    {
        helper->dumpStart[helper->depth-1] = i;
        if(!Param_AppendDumpFromHere(current->children[i], buffer, buffer_length, helper))
        {
            return 0;
        }
        if(!Param_AppendParentNotation(buffer, buffer_length))
        {
            return 0;
        }
    }
    helper->depth--;
    helper->dumpStart[helper->depth] = 0;
    return 1;
}

uint8_t Param_CompareValueType(param_obj_t *current, uint8_t *valueTypeBuffer)
{
    if(!current || !valueTypeBuffer)
    {
        return 0;
    }
    if(!strlen( (const char *)valueTypeBuffer))
    {
        return 1; // Ok to not have value string.
    }

    char *end;
    long int value = strtol((char *) valueTypeBuffer, &end, 10);
    if(value != current->type)
    {
        return 0;
    }
    return 1;

}

uint8_t Log_CompareName(param_obj_t *current, uint8_t *nameBuffer)
{
    if(!current || !nameBuffer)
    {
        return 0;
    }
    return !strncmp((char *)current->group_name, (char *)nameBuffer, MAX_PARAM_NAME_LENGTH);
}

uint8_t Param_UpdateCurrent(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBuffer_length)
{
    if(!current || !moduleBuffer)
    {
        return 0;
    }
    uint8_t bufferName[MAX_PARAM_NAME_LENGTH+1];
    uint8_t bufferValueType[MAX_VALUE_TYPE_LENGTH+1];
    uint8_t bufferValue[MAX_DIGITS_INT32+1];

    bufferName[0] = '\0';
    bufferValueType[0] = '\0';
    bufferValue[0] = '\0';
    // Make sure the string is null terminated.
    bufferName[MAX_PARAM_NAME_LENGTH] = '\0';
    bufferValueType[MAX_VALUE_TYPE_LENGTH] = '\0';
    bufferValue[MAX_DIGITS_INT32] = '\0';

    if(!FcString_GetNameString(bufferName, MAX_PARAM_NAME_LENGTH, moduleBuffer, moduleBuffer_length))
    {
        return 0;
    }

    FcString_GetValueTypeString(bufferValueType, MAX_VALUE_TYPE_LENGTH, moduleBuffer, moduleBuffer_length);
    FcString_GetValueString(bufferValue, MAX_DIGITS_INT32, moduleBuffer, moduleBuffer_length);
    //Make sure it is the right node, parent notation is ok.
    if(!Log_CompareName(current, bufferName) && (strncmp("..", (char *)bufferName,2)))
    {
        return 0;
    }
    //Only try to update if the node has the same value type as current (or no
    // value type in string).
    if(!Param_CompareValueType(current, bufferValueType))
    {
        return 0;
    }
    // Update the value of the current node if there is a value in string.
    if(!Param_SettNodeValue(current, bufferValue))
    {
        return 0;
    }
    return 1;
}

param_obj_t * Param_FindNext(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
    if(!current || !moduleBuffer)
    {
        return NULL;
    }
    uint8_t bufferName[MAX_PARAM_NAME_LENGTH];
    bufferName[0] = '\0';
    bufferName[MAX_PARAM_NAME_LENGTH -1] = '\0';

    //The buffer has to contain a valid name.
    if(!FcString_GetNameString(bufferName, MAX_PARAM_NAME_LENGTH , moduleBuffer, moduleBufferLength))
    {
        return NULL;
    }
    // Return parent if parent is requested.
    if(!strncmp("..", (char *)bufferName,2))
    {
        return current->parent;
    }
    // Find child with requested name.
    for(uint8_t i = 0; i < current->registered_children; i++)
    {

        if(Log_CompareName(current->children[i], bufferName))
        {
            return current->children[i];
        }
    }
    // If not found, return null indicating error.
    return NULL;
}

uint8_t Param_SetFromHere(param_obj_t *current, uint8_t *Buffer, uint32_t BufferLength)
{
    if(!current || !BufferLength || !Buffer)
    {
        return 0;
    }

    uint8_t * buffPtr = (Buffer);
    uint32_t buffPtrLength = (uint32_t)(BufferLength - (buffPtr - Buffer));
    uint8_t bufferModule[MAX_PARAM_NODE_LENGTH];

    param_obj_t * workingOnObj = current;

    // Update root if needed.
    buffPtr = FcString_GetModuleString(bufferModule, MAX_PARAM_NODE_LENGTH, buffPtr, buffPtrLength);
    if(!buffPtr)
    {
        return 0;
    }
    buffPtrLength = (uint32_t)(BufferLength - (buffPtr - Buffer));

    // Update the module.
    if(!Param_UpdateCurrent(current, bufferModule, MAX_PARAM_NODE_LENGTH))
    {
        return 0;
    }
    // Find and update according to the string in Buffer.
    while(buffPtrLength > 1)
    {
        // Get module string of next module to update.
        buffPtr = FcString_GetModuleString(bufferModule, MAX_PARAM_NODE_LENGTH, buffPtr, buffPtrLength);
        if(!buffPtr)
        {
            return 0;
        }
        if((buffPtr < Buffer))
        {
            return 0;
        }
        if(BufferLength < (buffPtr - Buffer))
        {
            return 0;
        }
        buffPtrLength = (uint32_t)(BufferLength - (buffPtr - Buffer));

        // Find the module.
        workingOnObj = Param_FindNext(workingOnObj, bufferModule, MAX_PARAM_NODE_LENGTH);
        if(!workingOnObj)
        {
            return 0; // Module not found.
        }
        // Update the module.
        if(!Param_UpdateCurrent(workingOnObj, bufferModule, MAX_PARAM_NODE_LENGTH))
        {
            return 0;
        }
    }
    return 1;
}


