/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UART_RC_1_RX_Pin GPIO_PIN_2
#define UART_RC_1_RX_GPIO_Port GPIOE
#define UART_RC_1_TX_Pin GPIO_PIN_3
#define UART_RC_1_TX_GPIO_Port GPIOE
#define RC_1_PWR_CTRL_Pin GPIO_PIN_4
#define RC_1_PWR_CTRL_GPIO_Port GPIOE
#define SPI_AUX_MISO_Pin GPIO_PIN_5
#define SPI_AUX_MISO_GPIO_Port GPIOE
#define SPI_AUX_MOSI_Pin GPIO_PIN_6
#define SPI_AUX_MOSI_GPIO_Port GPIOE
#define REV1_Pin GPIO_PIN_13
#define REV1_GPIO_Port GPIOC
#define REV2_Pin GPIO_PIN_14
#define REV2_GPIO_Port GPIOC
#define REV3_Pin GPIO_PIN_15
#define REV3_GPIO_Port GPIOC
#define IMU2_INT_Pin GPIO_PIN_0
#define IMU2_INT_GPIO_Port GPIOC
#define REV4_Pin GPIO_PIN_1
#define REV4_GPIO_Port GPIOC
#define SPI_SLAVE_MISO_Pin GPIO_PIN_2
#define SPI_SLAVE_MISO_GPIO_Port GPIOC
#define SPI_SLAVE_MOSI_Pin GPIO_PIN_3
#define SPI_SLAVE_MOSI_GPIO_Port GPIOC
#define ADC_BAT_Pin GPIO_PIN_0
#define ADC_BAT_GPIO_Port GPIOA
#define ADC_1_Pin GPIO_PIN_1
#define ADC_1_GPIO_Port GPIOA
#define ADC_2_Pin GPIO_PIN_2
#define ADC_2_GPIO_Port GPIOA
#define ADC_3_Pin GPIO_PIN_3
#define ADC_3_GPIO_Port GPIOA
#define SPI_MEM_NSS_Pin GPIO_PIN_4
#define SPI_MEM_NSS_GPIO_Port GPIOA
#define SPI_MEM_SCK_Pin GPIO_PIN_5
#define SPI_MEM_SCK_GPIO_Port GPIOA
#define SPI_MEM_MISO_Pin GPIO_PIN_6
#define SPI_MEM_MISO_GPIO_Port GPIOA
#define SPI_MEM_MOSO_Pin GPIO_PIN_7
#define SPI_MEM_MOSO_GPIO_Port GPIOA
#define IO_12_Pin GPIO_PIN_4
#define IO_12_GPIO_Port GPIOC
#define UART_COM_RX_Pin GPIO_PIN_5
#define UART_COM_RX_GPIO_Port GPIOC
#define PWM_5_Pin GPIO_PIN_0
#define PWM_5_GPIO_Port GPIOB
#define PWM_6_Pin GPIO_PIN_1
#define PWM_6_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define IO_1_Pin GPIO_PIN_7
#define IO_1_GPIO_Port GPIOE
#define IO_2_Pin GPIO_PIN_8
#define IO_2_GPIO_Port GPIOE
#define PWM_1_Pin GPIO_PIN_9
#define PWM_1_GPIO_Port GPIOE
#define IO_3_Pin GPIO_PIN_10
#define IO_3_GPIO_Port GPIOE
#define SPI_AUX_NSS_Pin GPIO_PIN_11
#define SPI_AUX_NSS_GPIO_Port GPIOE
#define SPI_AUX_SCK_Pin GPIO_PIN_12
#define SPI_AUX_SCK_GPIO_Port GPIOE
#define PWM_3_Pin GPIO_PIN_13
#define PWM_3_GPIO_Port GPIOE
#define PWM_4_Pin GPIO_PIN_14
#define PWM_4_GPIO_Port GPIOE
#define IO_4_Pin GPIO_PIN_15
#define IO_4_GPIO_Port GPIOE
#define NC1_Pin GPIO_PIN_10
#define NC1_GPIO_Port GPIOB
#define SPI_SLAVE_NSS_Pin GPIO_PIN_12
#define SPI_SLAVE_NSS_GPIO_Port GPIOB
#define SPI_SLAVE_SCK_Pin GPIO_PIN_13
#define SPI_SLAVE_SCK_GPIO_Port GPIOB
#define UART_COM_RTS_Pin GPIO_PIN_14
#define UART_COM_RTS_GPIO_Port GPIOB
#define IO_11_Pin GPIO_PIN_15
#define IO_11_GPIO_Port GPIOB
#define UART_COM_TX_Pin GPIO_PIN_8
#define UART_COM_TX_GPIO_Port GPIOD
#define IO_5_Pin GPIO_PIN_9
#define IO_5_GPIO_Port GPIOD
#define IO_6_Pin GPIO_PIN_10
#define IO_6_GPIO_Port GPIOD
#define UART_COM_CTS_Pin GPIO_PIN_11
#define UART_COM_CTS_GPIO_Port GPIOD
#define IO_7_Pin GPIO_PIN_12
#define IO_7_GPIO_Port GPIOD
#define IO_8_Pin GPIO_PIN_13
#define IO_8_GPIO_Port GPIOD
#define UART_DBG_RX_Pin GPIO_PIN_14
#define UART_DBG_RX_GPIO_Port GPIOD
#define UART_DBG_TX_Pin GPIO_PIN_15
#define UART_DBG_TX_GPIO_Port GPIOD
#define UART_MOTOR_2_TX_Pin GPIO_PIN_6
#define UART_MOTOR_2_TX_GPIO_Port GPIOC
#define UART_MOTOR_2_RX_Pin GPIO_PIN_7
#define UART_MOTOR_2_RX_GPIO_Port GPIOC
#define IO_13_Pin GPIO_PIN_8
#define IO_13_GPIO_Port GPIOC
#define IO_14_Pin GPIO_PIN_9
#define IO_14_GPIO_Port GPIOC
#define IO_9_Pin GPIO_PIN_8
#define IO_9_GPIO_Port GPIOA
#define PWM_2_Pin GPIO_PIN_9
#define PWM_2_GPIO_Port GPIOA
#define UART_COM_USB_RX_Pin GPIO_PIN_10
#define UART_COM_USB_RX_GPIO_Port GPIOA
#define UART_COM_USB_CTS_Pin GPIO_PIN_11
#define UART_COM_USB_CTS_GPIO_Port GPIOA
#define UART_COM_USB_RTS_Pin GPIO_PIN_12
#define UART_COM_USB_RTS_GPIO_Port GPIOA
#define UART_COM_USB_TX_Pin GPIO_PIN_15
#define UART_COM_USB_TX_GPIO_Port GPIOA
#define IO_15_Pin GPIO_PIN_10
#define IO_15_GPIO_Port GPIOC
#define IO_10_Pin GPIO_PIN_11
#define IO_10_GPIO_Port GPIOC
#define HEARTBEAT_Pin GPIO_PIN_12
#define HEARTBEAT_GPIO_Port GPIOC
#define LED_SETPOINT_Pin GPIO_PIN_0
#define LED_SETPOINT_GPIO_Port GPIOD
#define LED_CTRL_MODE_Pin GPIO_PIN_1
#define LED_CTRL_MODE_GPIO_Port GPIOD
#define LED_STATUS_Pin GPIO_PIN_2
#define LED_STATUS_GPIO_Port GPIOD
#define LED_ERROR_Pin GPIO_PIN_3
#define LED_ERROR_GPIO_Port GPIOD
#define LED_FATAL_Pin GPIO_PIN_4
#define LED_FATAL_GPIO_Port GPIOD
#define UART_RC_2_TX_Pin GPIO_PIN_5
#define UART_RC_2_TX_GPIO_Port GPIOD
#define UART_RC_2_RX_Pin GPIO_PIN_6
#define UART_RC_2_RX_GPIO_Port GPIOD
#define RC_2_PWR_CTRL_Pin GPIO_PIN_7
#define RC_2_PWR_CTRL_GPIO_Port GPIOD
#define PWM_7_Pin GPIO_PIN_4
#define PWM_7_GPIO_Port GPIOB
#define PWM_8_Pin GPIO_PIN_5
#define PWM_8_GPIO_Port GPIOB
#define I2C_SCL_IMU_Pin GPIO_PIN_6
#define I2C_SCL_IMU_GPIO_Port GPIOB
#define I2C_SDA_IMU_Pin GPIO_PIN_7
#define I2C_SDA_IMU_GPIO_Port GPIOB
#define IMU1_INT_Pin GPIO_PIN_8
#define IMU1_INT_GPIO_Port GPIOB
#define IMU1_INT_EXTI_IRQn EXTI9_5_IRQn
#define NC_Pin GPIO_PIN_9
#define NC_GPIO_Port GPIOB
#define UART_MOTOR_1_RX_Pin GPIO_PIN_0
#define UART_MOTOR_1_RX_GPIO_Port GPIOE
#define UART_MOTOR_1_RXE1_Pin GPIO_PIN_1
#define UART_MOTOR_1_RXE1_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
