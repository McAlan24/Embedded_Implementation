/**
  ******************************************************************************
  * @file           : sensors_monitor.h
  * @brief          : Header for sensors monitoring system
  ******************************************************************************
  * @attention
  *
  * Sensors Monitoring System for STM32H563ZI
  * Handles auto-monitoring for all sensors
  *
  ******************************************************************************
  */

#ifndef __SENSORS_MONITOR_H
#define __SENSORS_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "hdc1080.h"
#include "adxl345.h"

/* Exported types ------------------------------------------------------------*/

/* Sensors Monitor Configuration */
typedef struct {
    uint32_t last_hdc1080_update;
    uint32_t last_adxl345_update;
    uint8_t monitoring_active;
    uint32_t total_readings;
} sensors_monitor_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Sensor Monitoring Functions */
void SensorsMonitor_Init(void);
void SensorsMonitor_Update(void);
void SensorsMonitor_SetActive(uint8_t active);
uint8_t SensorsMonitor_IsActive(void);
void SensorsMonitor_PrintStatus(void);

/* Individual Sensor Updates */
void SensorsMonitor_UpdateHDC1080(void);
void SensorsMonitor_UpdateADXL345(void);

/* Sensor Data Access */
sensors_monitor_t* SensorsMonitor_GetContext(void);

/* Shell Commands */
shell_status_t cmd_sensors_monitor(int argc, char **argv);
shell_status_t cmd_sensors_status(int argc, char **argv);
shell_status_t cmd_sensors_all(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* __SENSORS_MONITOR_H */
