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
#include "Modules/Parameters/inc/parameters.h"


#include <gpio.h>
#include <pio.h>
static param_obj_t * mRoot = NULL;

/**
 * @brief Initialize the param system by creating the root node.
 */
void Param_Init();


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
 * @param current	The current node.
 * @param parent	Parent node of current.
 */
void Param_SetParent(param_obj_t *current, param_obj_t *parent);

/**
 * @brief Get parent node.
 *
 * Gets the parent node of current. Returns NULL if no parent is registerd.
 *
 * @param current 	Current node.
 * @return			Pointer to parent, or null if no parent exists.
 */
param_obj_t *Param_GetParent(param_obj_t *current);

/**
 * @brief Get child nr index of current.
 *
 * Get child from current node. If index is larger than the registered number
 * of children, then the function will return NULL.
 *
 * @param current 	Current node.
 * @param index		Index of child to return.
 * @return			Pointer to child. NULL if no child with that index exist.
 */
param_obj_t *Param_GetChild(param_obj_t *current, uint8_t index);

/**
 * @brief Append divider "/"
 *
 * Append the divider "/" to the buffer after doing a boundary check.
 *
 * @param buffer		Buffer to append to.
 * @param buffer_length Buffer length, used for boundary check.
 * @return				0 if fail, 1 otherwise.
 */
uint8_t Param_AppendDivider( uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Append parent notation "../"
 *
 * Appends the parent notation to buffer after doing a boundary check. A trailing divider
 * will also be appended.
 *
 * @param buffer			Buffer to append to.
 * @param buffer_length		Buffer length, used for boundary check.
 * @return					0 if fail, 1 otherwise.
 */
uint8_t Param_AppendParentNotation( uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Get value from current node.
 *
 * Gets the value of the current node, represented as ascii. Will write to the
 * start of buffer. Null terminates the string.
 *
 * @param current		Current node.
 * @param buffer		Buffer to write the value to. Writes to the start of buffer.
 * @param buffer_length	Length of buffer, used for boundary check.
 * @return				0 if fail, 1 otherwise.
 */
uint8_t Param_GetNodeValue(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Set the node value of current.
 *
 * Set the value of the current node. The value is represented as ascii in buffer.
 *
 * @param current	Current node.
 * @param buffer	Buffer containing the value.
 * @return			0 if fail, 1 otherwise.
 */
uint8_t Param_SettNodeValue(param_obj_t *current, uint8_t *buffer);

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
 * @param StringBuffer			Input string buffer.
 * @param stringBufferLength	Input string buffer length.
 * @param buffer				Outout buffer.
 * @param buffer_length			Outout buffer length.
 * @param start					Start char.
 * @param end					End char.
 * @return						Pointer to first element after "end". Null if fail.
 */
uint8_t *Param_GetStringBetween(uint8_t *StringBuffer, uint32_t stringBufferLength
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
 * @param toBuffer			Buffer to copy to.
 * @param toBufferLength	Length of buffer to copy to, used for boundary check.
 * @param fromBuffer		Buffer to search for delimiter in, and copy from.
 * @param frombufferLength	Length of from buffer, used for boundary check.
 * @param delimiters		String of delimiters to search for.
 * @param noDelimiters		Number of delimiters to search for.
 * @return					pointer to first occurence. Null if fail.
 */
uint8_t *Param_GetStringUntil(uint8_t *toBuffer, uint32_t toBufferLength, uint8_t *fromBuffer, uint32_t frombufferLength
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
 * @param valueBuffer			Buffer to write the value to as ascii.
 * @param valueBufferLength		Length of value buffer, used for boundary check.
 * @param moduleBuffer			Buffer containing the module string copied from.
 * @param moduleBufferLength	Length of module buffer, used for boundary check.
 * @return						Pointer to first element after end of value string. Null if fail.
 */
uint8_t *Param_GetValueString(uint8_t *valueBuffer, uint32_t valueBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

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
 * @param valueTypeBuffer			Buffer to write the value type to as ascii.
 * @param valueTypeBufferLength		Length of value type buffer, used for boundary check.
 * @param moduleBuffer				Buffer containing the module string copied from.
 * @param moduleBufferLength		Length of module buffer, used for boundary check.
 * @return							Pointer to first element after end of value type string. Null if fail.
 */
uint8_t *Param_GetValueTypeString(uint8_t *valueTypeBuffer, uint32_t valueTypeBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

/**
 * @brief Get module string.
 *
 * Get the module string from the path described in pathBuffer. Will not include
 * delimiters.
 *
 * @param moduleBuffer			Buffer to copy module string to.
 * @param moduleBufferLength	Length of module buffer, used for boundary check.
 * @param pathBuffer			Buffer containing the path to be handled.
 * @param PathBufferLength		Length of pathBuffer, used for boundary check.
 * @return						Pointer to first element after module string. Null if fail.
 */
uint8_t *Param_GetModuleString(uint8_t *moduleBuffer, uint32_t moduleBufferLength, uint8_t *pathBuffer, uint32_t PathBufferLength);

/**
 * @brief Get name string from module string.
 *
 * Get the name string from the module string in moduleBuffer. Name will be placed
 * at the beginning of nameBuffer.
 *
 * @param nameBuffer			Buffer to copy the name to.
 * @param nameBufferLength		Length of name buffer, used for boundary check.
 * @param moduleBuffer			Buffer containing the module string.
 * @param moduleBufferLength 	module buffer length, used for boundary check.
 * @return						Pointer to first element after name sting.
 */
uint8_t *Param_GetNameString(uint8_t *nameBuffer, uint32_t nameBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

/**
 * @brief Append full node string to buffer.
 *
 * Appends current node represented as a string to the end of the null terminated buffer.
 *
 * Will fail if the buffer is not long enough.
 *
 * @param current		Current node.
 * @param buffer		Buffer to append to.
 * @param buffer_length	Total length of buffer, used for boundary check.
 * @return				0 if fail, 1 otherwise.
 */
uint8_t Param_AppendNodeString(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Append path to current node.
 *
 * Append path from root to and including current node to buffer.
 *
 * Will fail if buffer is not long enough.
 *
 * @param current		Current node.
 * @param buffer		Buffer to append to.
 * @param buffer_length	Total length of buffer, used for boundary check
 * @return				0 if fail, 1 otherwise.
 */
uint8_t Param_AppendPathToHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Compare value type of current with value type in buffer.
 *
 * Compares value type of current with the value type in valueTypeBuffer.
 *
 * @param current			Current node.
 * @param valueTypeBuffer	Buffer containing the value type.
 * @return					0 if fail, 1 otherwise.
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
 * @param current				Current node.
 * @param moduleBuffer			Buffer containing the module string used to update with.
 * @param moduleBuffer_length	Length of module buffer, used for boundary check.
 * @return						0 if fail, 1 if otherwise.
 */
uint8_t Param_UpdateCurrent(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBuffer_length);

/**
 * @brief Find the module in moduleBuffer.
 *
 * Find the next node, described in moduleBuffer. Will find children or
 * parent of current node.
 *
 * @param current				Current node.
 * @param moduleBuffer			Buffer containing the node to find.
 * @param moduleBufferLength	Length of buffer, used for boundary check.
 * @return						Pointer to found object. Null if fail.
 */
param_obj_t * Param_FindNext(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBufferLength);

void Param_Init()
{
  mRoot = Param_CreateObj(12, NoType, readOnly, NULL, "QuadFC", NULL, NULL );
}

param_obj_t *Param_GetRoot()
{
  if(!mRoot)
  {
    Param_Init();
  }
  return mRoot;
}

void Param_SetChild(param_obj_t *current, param_obj_t *child)
{
  if(!current || !child || !(current->registered_children < current->num_variables))
  {
    return; //TODO error
  }
  current->children[current->registered_children++] = child;
}

param_obj_t *Param_CreateObj(uint8_t num_children, Log_variable_type_t type,
    Log_variable_access_t access, void *value, const char *obj_name,
    param_obj_t *parent, SemaphoreHandle_t xMutex)
{

  if((NoType != type) && !value)
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
  log_obj->group_name = pvPortMalloc( sizeof( unsigned char ) * MAX_LOG_NAME_LENGTH );
  log_obj->parent = parent;
  log_obj->registered_children = 0;
  log_obj->xMutex = xMutex;
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
  strncpy( (char *) log_obj->group_name , obj_name, (unsigned short) MAX_LOG_NAME_LENGTH - 1);
  log_obj->group_name[MAX_LOG_NAME_LENGTH] = '\0';

  if(parent)
  {
    Param_SetChild(parent, log_obj);
  }
  return log_obj;
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
  strncat( (char *) buffer, (const char *) "/", (unsigned short) 1);
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
  strncat( (char *) buffer, (const char *) "../", (unsigned short) 3);
  return 1;
}

uint8_t Param_GetNodeValue(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length)
{
  if(!current || !buffer || (buffer_length < MAX_DIGITS_INT32))
  {
    return 0;
  }

  // Aquire the mutex if there is one connected to current.
  if(current->xMutex)
  {
    if( !(xSemaphoreTake( current->xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE) )
    {
      return 0; // Was not able to aquire mutex, return with error.
    }
  }

  uint8_t result = 0;

  uint8_t pTemp[11];
  switch (current->type)
  {
  case NoType:
    break;
  case uint8_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%hu", (*(uint8_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case uint16_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%u", (*(uint16_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case uint32_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%lu", (*(uint32_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case int8_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%hi", (*(int8_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case int16_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%i", (*(int16_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case int32_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%li", (*(int32_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case fp_16_16_variable_type:
    snprintf((char *) pTemp, MAX_DIGITS_INT32, "%li", (*(int32_t*)current->value));
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_DIGITS_INT32);
    result = 1;
    break;
  case last_variable_type:
    break;
  default:
    break;
  }
  if(current->xMutex)
  {
    xSemaphoreGive(current->xMutex);
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

  // Aquire the mutex to ensure thread saftey.
  if(current->xMutex)
  {
    if( !(xSemaphoreTake( current->xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE) )
    {
      return 0; // Was not able to aquire mutex, return with error.
    }
  }
  uint8_t result = 0;
  char *end;
  long int value = strtol((char *) buffer, &end, 10);

  switch (current->type)
  {
  case NoType:
    break;
  case uint8_variable_type:
    if(value < UINT8_MAX)
    {
      *((uint8_t*)current->value) = (uint8_t)value;
      result = 1;
    }
    break;
  case uint16_variable_type:
    if(value < UINT16_MAX)
    {
      *((uint16_t*)current->value) = (uint16_t)value;
      result = 1;
    }
    break;
  case uint32_variable_type:
    if(value < UINT32_MAX)
    {
      *((uint32_t*)current->value) = (uint32_t)value;
      result = 1;
    }
    break;
  case int8_variable_type:
    if(value < INT8_MAX)
    {
      *((int8_t*)current->value) = (int8_t)value;
      result = 1;
    }
    break;
  case int16_variable_type:
    if(value < INT16_MAX)
    {
      *((int16_t*)current->value) = (int16_t)value;
      result = 1;
    }
    break;
  case int32_variable_type:
    if(value < INT32_MAX)
    {
      *((int32_t*)current->value) = (int32_t)value;
      result = 1;
    }
    break;
  case fp_16_16_variable_type:
    if(value < INT32_MAX)
    {
      *((int32_t*)current->value) = (int32_t)value;
      result = 1;
    }
    break;
  case last_variable_type:
    break;
  default:
    break;
  }
  if(current->xMutex)
  {
    xSemaphoreGive(current->xMutex);
  }
  return result;
}

uint8_t *Param_GetStringBetween(uint8_t *StringBuffer, uint32_t stringBufferLength
    , uint8_t *buffer, uint32_t buffer_length
    , const char start, const char end)
{
  StringBuffer[0] = '\0';
  if(!StringBuffer || !buffer || !start || !end)
  {
    return NULL;
  }
  char * matchFront;
  char * matchBack;
  //Find first occurence of "/" in buffer.
  matchFront = strchr ((char *)buffer, (int)start);
  matchBack = strchr ((char *)buffer, (int)end);


  //Make sure it is a valid match, and that the match fits in the provided buffer.
  if(!matchFront
      || !matchBack
      || (matchFront > matchBack )
      || (((matchBack - matchFront +1) ) > stringBufferLength))
  {
    return NULL;
  }

  uint32_t length = (uint32_t)(matchBack - matchFront);

  // Copy the value string (everything between "matchFront" and "matchBack") to value buffer,
  // discarding the indicators. Make sure that we only copy valid data, and that the data will fit
  // in target.
  if(length > stringBufferLength || (length > (buffer_length - ((char *)buffer - matchFront))))
  {
    return NULL;
  }
  strncpy ((char *)StringBuffer, (matchFront+1), length-1);
  StringBuffer[ length - 1] = '\0';

  // Return pointer to character after match.
  return (uint8_t *)(matchBack + 1);
}

uint8_t *Param_GetStringUntil(uint8_t *toBuffer, uint32_t toBufferLength, uint8_t *fromBuffer, uint32_t frombufferLength
    , const char* delimiters, uint32_t noDelimiters)
{
  toBuffer[0]= '\0';
  if(!toBuffer || !fromBuffer || !delimiters)
  {
    return NULL;
  }
  char * firstOccurence = NULL;

  //Find first occurence of the delimiters.
  for (uint32_t i = 0; i < noDelimiters; i++)
  {
    char * current = strchr ((char *)fromBuffer, (int)delimiters[i]);
    if(!firstOccurence)
    {
      firstOccurence = current;
    }
    else if (current && (current < firstOccurence))
    {
      firstOccurence = current;
    }
  }

  if(!firstOccurence || firstOccurence < (char *)fromBuffer || (firstOccurence > (char *)(fromBuffer + frombufferLength)))
  {
    return NULL;
  }
  uint32_t length = (uint32_t)(firstOccurence - (char *)fromBuffer);
  // target has to be long enough for text + null terminaing.
  if((( length + 1 ) > toBufferLength))
  {
    return NULL;
  }
  //Copy length number of bytes to target, omitting the matched char.
  strncpy ((char *)toBuffer, (char *)fromBuffer, length);
  toBuffer[ length ] = '\0';
  // Return pointer to character after match.
  return (uint8_t *)(firstOccurence);
}

uint8_t *Param_GetValueString(uint8_t *valueBuffer, uint32_t valueBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  return Param_GetStringBetween(valueBuffer, valueBufferLength, moduleBuffer, moduleBufferLength, '[', ']');
}

uint8_t *Param_GetValueTypeString(uint8_t *valueTypeBuffer, uint32_t valueTypeBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  return Param_GetStringBetween(valueTypeBuffer, valueTypeBufferLength, moduleBuffer, moduleBufferLength, '<', '>');
}

uint8_t *Param_GetModuleString(uint8_t *moduleBuffer, uint32_t moduleBufferLength, uint8_t *pathBuffer, uint32_t PathBufferLength)
{
  const char delimiters[]={"/\0"};
  uint8_t * after_string = Param_GetStringUntil(moduleBuffer, moduleBufferLength, pathBuffer, PathBufferLength, delimiters, 1);
  if(!after_string)
  {
    return NULL;
  }
  return (after_string + 1);
}

uint8_t *Param_GetNameString(uint8_t *nameBuffer, uint32_t nameBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  const char delimiters[]={'<','[','/','\0'};
  uint8_t * after_string = Param_GetStringUntil(nameBuffer, nameBufferLength, moduleBuffer, moduleBufferLength, delimiters, 3);
  if(!after_string)
  {
    // Found no match for delimiters in module string, module string contains only name.
    uint32_t length = 0;
    // Make sure that the string in moduleBuffer can fit in nameBuffer.Do not copy more than what is left in moduleBuffer.
    uint32_t stringLength = (uint32_t)strlen((char *)moduleBuffer);
    length = ((nameBufferLength < moduleBufferLength) ? nameBufferLength : moduleBufferLength);
    if(stringLength+1 > length)
    {
      return NULL;
    }

    strncpy ((char *)nameBuffer, (char *)moduleBuffer, stringLength+1);
    return (nameBuffer + stringLength);
  }
  return (after_string);
}

uint8_t Param_AppendNodeString(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length)
{
  if(!buffer || !current)
  {
    return 0;
  }
  uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);

  //ensure that the name will fit in current buffer.
  if((buf_index + MAX_NODE_LENGTH) > buffer_length)
  {
    return 0;
  }
  //append group name and update buf_index.
  strncat( (char *) buffer , (const char *) current->group_name, (unsigned short) MAX_LOG_NAME_LENGTH);
  // append variabletype "<y>" and value "[xxx]"
  strncat( (char *) buffer, (const char *) "<", (unsigned short) 1);
  uint8_t pTemp[MAX_VALUE_TYPE_LENGTH];
  snprintf((char *) pTemp, MAX_VALUE_TYPE_LENGTH, "%lu",(uint32_t)current->type);
  strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_VALUE_TYPE_LENGTH);
  strncat( (char *) buffer, (const char *) ">", (unsigned short) 1);
  if(NoType != current->type)
  {
    strncat( (char *) buffer, (const char *) "[", (unsigned short) 1);
    if(!Param_GetNodeValue(current, buffer, buffer_length))
    {
      return 0;
    }
    strncat( (char *) buffer, (const char *) "]", (unsigned short) 1);
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
  if((buf_index + ((MAX_NODE_LENGTH + 1) * parentIdx) ) > buffer_length)
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

uint8_t Param_DumpFromRoot(uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper)
{
  buffer[0] = '\0';
  if(!Param_AppendDivider( buffer, buffer_length))
  {
    return 0;
  }
  helper->depth = 0;
  return Param_AppendDumpFromHere(Param_GetRoot(), buffer, buffer_length, helper);
}

uint8_t Param_AppendDumpFromHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper)
{
  if(!buffer || !current || (helper->depth > MAX_DEPTH))
  {
    return 0;
  }
  //ensure that the name will fit in current buffer.

  uint32_t buf_index = (uint32_t)strlen( (const char *)buffer);
  if((buf_index + MAX_NODE_LENGTH) > buffer_length)
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
  return !strcmp((char *)current->group_name, (char *)nameBuffer);
}

uint8_t Param_UpdateCurrent(param_obj_t *current, uint8_t *moduleBuffer, uint32_t moduleBuffer_length)
{
  if(!current || !moduleBuffer)
  {
    return 0;
  }
  uint8_t bufferName[MAX_LOG_NAME_LENGTH+1];
  uint8_t bufferValueType[MAX_VALUE_TYPE_LENGTH+1];
  uint8_t bufferValue[MAX_DIGITS_INT32+1];

  bufferName[0] = '\0';
  bufferValueType[0] = '\0';
  bufferValue[0] = '\0';
  // Make sure the string is null terminated.
  bufferName[MAX_LOG_NAME_LENGTH] = '\0';
  bufferValueType[MAX_VALUE_TYPE_LENGTH] = '\0';
  bufferValue[MAX_DIGITS_INT32] = '\0';

  if(!Param_GetNameString(bufferName, MAX_LOG_NAME_LENGTH, moduleBuffer, moduleBuffer_length))
  {
    return 0;
  }

  Param_GetValueTypeString(bufferValueType, MAX_VALUE_TYPE_LENGTH, moduleBuffer, moduleBuffer_length);
  Param_GetValueString(bufferValue, MAX_DIGITS_INT32, moduleBuffer, moduleBuffer_length);
  //Make sure it is the right node, parent notation is ok.
  if(!Log_CompareName(current, bufferName) && (strcmp("..", (char *)bufferName)))
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
  uint8_t bufferName[MAX_LOG_NAME_LENGTH];
  bufferName[0] = '\0';
  bufferName[MAX_LOG_NAME_LENGTH -1] = '\0';

  //The buffer has to contain a valid name.
  if(!Param_GetNameString(bufferName, MAX_LOG_NAME_LENGTH , moduleBuffer, moduleBufferLength))
  {
    return NULL;
  }
  // Return parent if parent is requested.
  if(!strcmp("..", (char *)bufferName))
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

uint8_t Param_SetFromRoot(param_obj_t *current, uint8_t *Buffer, uint32_t BufferLength)
{
  if(!current || !BufferLength || !Buffer)
  {
    return 0;
  }
  // First char of the buffer has to indicate that the path is relative root.
  if(Buffer[0] != '/')
  {
    return 0;
  }
  uint8_t * buffPtr = (Buffer + 1);
  uint32_t buffPtrLength = (uint32_t)(BufferLength - (buffPtr - Buffer));
  uint8_t bufferModule[MAX_NODE_LENGTH];

  param_obj_t * workingOnObj = current;

  // Update root if needed.
  buffPtr = Param_GetModuleString(bufferModule, MAX_NODE_LENGTH, buffPtr, buffPtrLength);
  if(!buffPtr)
  {
    return 0;
  }
  buffPtrLength = (uint32_t)(BufferLength - (buffPtr - Buffer));

  // Update the module.
  if(!Param_UpdateCurrent(current, bufferModule, MAX_NODE_LENGTH))
  {
    return 0;
  }
  // Find and update according to the string in Buffer.
  while(buffPtrLength > 1)
  {
    // Get module string of next module to update.
    buffPtr = Param_GetModuleString(bufferModule, MAX_NODE_LENGTH, buffPtr, buffPtrLength);
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
    workingOnObj = Param_FindNext(workingOnObj, bufferModule, MAX_NODE_LENGTH);
    if(!workingOnObj)
    {
      return 0;
    }
    // Update the module.
    if(!Param_UpdateCurrent(workingOnObj, bufferModule, MAX_NODE_LENGTH))
    {
      return 0;
    }
  }
  return 1;
}


