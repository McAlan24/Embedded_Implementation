/**
  ******************************************************************************
  * @file           : system_config.h
  * @brief          : System configuration and common definitions
  ******************************************************************************
  * @attention
  *
  * System Configuration Module for STM32H563ZI
  * Contains common definitions and system variables
  *
  ******************************************************************************
  */

#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shell_terminal.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* System Functions */
void System_Init(void);
void System_Process(void);

/* System Command */
shell_status_t cmd_button(int argc, char **argv);
shell_status_t cmd_display(int argc, char **argv);
shell_status_t cmd_screen(int argc, char **argv);

/* System Variables Access */
extern volatile uint32_t button_press_count;
extern volatile uint32_t last_interrupt_time;
extern uint32_t system_boot_time;

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CONFIG_H */
