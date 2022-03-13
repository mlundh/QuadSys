/**
 * @file parameters.h
 *
 * Module implementing parameters. Parameters can be accessed and written to by interface
 * functions. These functions uses arrays of uint8_t to describe the tree. The interface
 * relies on the following format:
 *
 * / = separator
 * .. = parent
 * . = self
 * <xx> = value type where xx is the type
 * [zz] = value where zz is the value.
 *
 * A node will be represented in the following format:
 * 'name<xx>[zz]'
 *
 * If the value type is no_value then there is no value to report, and thus that field should
 * be left out:
 * 'name<0>'
 *
 * The value type can be omitted when setting a value:
 * 'name[65]'
 *
 * All interaction with the parameters will always start from the root node, and thus
 * always start with a separator. After the initial separator will be the root node
 * followed by all nodes affected by the interaction.
 *
 * @example If the tree has the following structure:
 *
 * root<0>
 *    rate<0>
 *        Kp<1>               [55]
 *        Ki<1>               [8]
 *        Kd<1>               [3]
 *    attitude<0>
 *        Kp<1>               [35]
 *        Ki<1>               [2]
 *        kd<1>               [12]
 *
 * Then a dump from root will look like:
 * /root<0>/rate<0>/Kp<1>[55]/../Ki<1>[8]/../Kd<1>[3]/../../attitude<0>/Kp<1>[35]/../Ki<1>[2]/../kd<1>[12]/../../
 *
 * And to set rate/Ki to '12' instead of '8' then the following string should be passed to the setter:
 * /root/rate/Ki<1>[12]
 *
 * it is also possible to omit the value type:
 * /root/rate/Ki[13]
 *
 *
 * All parameters should be created before the scheduler is started.
 *
 *  Created on: Jul 24, 2015
 *      Author: martin
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_


#include "FreeRTOS.h"
#include "semphr.h"


/**
 * Definitions used by the module.
 */
#define MAX_PARAM_NAME_LENGTH (17) // 16 bytes for name, 1 for null.
#define MAX_EXTRA (5)
#define MAX_DIGITS_INT32 (11)
#define MAX_VALUE_TYPE_LENGTH (4)
#define MAX_DEPTH (8)
#define MAX_PARAM_NODE_LENGTH (MAX_DIGITS_INT32 + MAX_EXTRA + MAX_PARAM_NAME_LENGTH + MAX_VALUE_TYPE_LENGTH)


/**
 * @enum varaiable_type
 * @brief Types of nodes allowed.
 *
 * The enum contains the supported value types for the parameters.
 */
typedef enum variable_type
{
    variable_type_NoType =            0,//!< variable_type_NoType
    variable_type_uint8  =            1,//!< uint8_variable_type
    variable_type_uint16 =            2,//!< uint16_variable_type
    variable_type_uint32 =            3,//!< uint32_variable_type
    variable_type_int8   =            4,//!< int8_variable_type
    variable_type_int16  =            5,//!< int16_variable_type
    variable_type_int32  =            6,//!< int32_variable_type
    variable_type_fp_16_16  =         7,//!< fp_16_16_variable_type
    variable_type_last  =             8,//!< last_variable_type
}Log_variable_type_t;

typedef enum variable_access
{
    readWrite               =            0,//!< read and write access
    readOnly                =            1,//!< read only
}Log_variable_access_t;

/**
 * @brief Callback for any param write operation.
 * 
 */
typedef void (*paramCB)(void* data);

/**
 * @struct log_obj_t
 * @brief The log objects.
 *
 * Struct containing all fields used by the parameter access methods.
 */
struct param_obj_t
{
    uint8_t num_variables;
    Log_variable_type_t type;
    Log_variable_access_t access;
    void* value;
    signed char *group_name;
    struct param_obj_t *parent;
    struct param_obj_t **children;
    uint8_t registered_children;
    paramCB callback;
    void* cbData;
} ;
typedef struct param_obj_t param_obj_t;

/**
 * Helper struct to save state of an aborted dump. This enables
 * a continued dump later.
 */
struct param_helper
{
    uint8_t dumpStart[MAX_DEPTH];
    uint8_t depth;
    uint8_t sequence;
};
typedef struct param_helper param_helper_t;


/**
 * @brief Creates a parameter object.
 *
 * Creates a parameter object with the given properties. This function
 * allocates enough memory for the object name and for the list
 * of children. To be able to do this it is necessary to give the maximum
 * number of children to a node.
 *
 * A node should always have a parent unless it is the root node. There should
 * only be one root node in each tree.
 *
 * @param num_children  Max number of children.
 * @param type				  Type of value, see variable_type enum.
 * @param value				  Pointer to the value that will become a parameter.
 * @param obj_name		  Name of the object/parameter.
 * @param parent			  Parent of object, NULL if object does not have a parent.

 * @return					    A handle to the newly created log object. NULL if
 * 							something went wrong.
 */
param_obj_t *Param_CreateObj(uint8_t num_children, Log_variable_type_t type,
        Log_variable_access_t access, void *value, const char *obj_name,
        param_obj_t *parent);

/**
 * Delete a param object. Use this with care as it does not de-register with parent or children.
 * @param obj
 */
void Param_DeleteObj(param_obj_t* obj);

void Param_SetCB(param_obj_t* obj, paramCB cb, void* cbData);


/**
 * @brief Append dump of current to buffer.
 *
 * Append path to, and children of, current node to the buffer. If current node is a root
 * node then the whole tree will be appended to buffer.
 *
 * The dump will be formatted according to the definition in the header of this file.
 *
 *
 * @param current			Node to start dump from.
 * @param buffer			Null terminated buffer to append the dump to.
 * @param buffer_length		Total length of the buffer.
 * @return					0 if fail, 1 if successful.
 */
uint8_t Param_AppendDumpFromHere(param_obj_t *current, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper);

/**
 * @brief Update tree from root with content of Buffer.
 *
 * Update the tree with the content of Buffer. Buffer should be formatted according to definition
 * in the header of this file.
 *
 * @param current
 * @param Buffer
 * @param BufferLength
 * @return
 */
uint8_t Param_SetFromHere(param_obj_t *current, uint8_t *Buffer, uint32_t BufferLength);



#endif /* PARAMETERS_H_ */
