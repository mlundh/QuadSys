/*
 * mk_esc.h
 *
 * Created: 2013-03-19 20:17:53
 *  Author: Martin Lundh
 */

#ifndef MKESC_H_
#define MKESC_H_

#include "motor_control.h"
#include "stdint.h"
#include "stddef.h"
#include "twi_master.h"
#include "freertos_twi_master.h"

#include "globals.h"


/*
 * Struct describing key properties and parameters for the BL_CTRL.
 *
 * *Note*
 * Please note that "runtime_data" contains information about:
 * [Current, max_pwm, temperature] in said order,
 * where Current is reported in 0.1 A steps, max_pwm is a status
 * showing: less than 255 if BL_CTRL is in current limit, not running (250)
 * or starting(40), and temperature is reported in �C
 */

#define BLOCK_TIME_TRANSMIT (2 / portTICK_RATE_ONE_THIRD_MS)// 1 tick timeout gives a timeout 
//between 0 and 1 tickperiods. Thus,
//always use at least 2 ticks timeouts.

typedef struct mk_esc_data
{
  twi_package_t twi_data;         //twi package
  xSemaphoreHandle twi_notification_semaphore;    //Notification semaphore
  freertos_twi_if twi;            // Initialized twi buss.
  portTickType xtransmit_block_time;              // block time
  uint8_t verified;              // != 0 if BL_CTRL is connected and responsive.
  uint8_t setpoint_transmitt[2];            // Setpoint value 11 bits for MK BL_CTRL v2.
  uint8_t runtime_data[3];        // data reported by BL_CTRL v2 in above order.
} mk_esc_data_t;

typedef struct mk_esc_motors
{
  mk_esc_data_t motor[MAX_MOTORS];
} mk_esc_motors_t;

#define BITCONF_REVERSE_ROTATION 0x01
#define BITCONF_RES1 0x02
#define BITCONF_RES2 0x04
#define BITCONF_RES3 0x08
#define BITCONF_RES4 0x10
#define BITCONF_RES5 0x20
#define BITCONF_RES6 0x40
#define BITCONF_RES7 0x80

/*The data in this struct must have exactly this order.*/
typedef struct mk_esc_config
{
  uint8_t Revision;                       // must be BL_REVISION
  uint8_t SetMask;                        // settings mask
  uint8_t PwmScaling;                     // maximum value of control pwm, acts like a thrust limit
  uint8_t CurrentLimit;                   // current limit in A
  uint8_t TempLimit;                      // in �C
  uint8_t CurrentScaling;                 // scaling factor for current measurement
  uint8_t BitConfig;                      // see defines above
  uint8_t crc;                            // checksum
}__attribute__((packed)) mk_esc_config_t;

void mk_esc_scale_11_bit_setpoint( int32_t data, uint8_t test[2] );

#endif /* MKESC_H_ */
