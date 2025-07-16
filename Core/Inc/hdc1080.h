/**
  ******************************************************************************
  * @file           : hdc1080.h
  * @brief          : Header for HDC1080 temperature/humidity sensor functions
  ******************************************************************************
  * @attention
  *
  * HDC1080 Temperature/Humidity Sensor Library for STM32H563ZI
  * Supports temperature and humidity measurements via I2C
  *
  ******************************************************************************
  */

#ifndef __HDC1080_H
#define __HDC1080_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "shell_terminal.h"

/* Exported types ------------------------------------------------------------*/

/* HDC1080 Sensor Configuration */
typedef struct {
    float temperature;      // Temperature in Celsius
    float humidity;         // Relative humidity in %
    uint16_t raw_temp;      // Raw temperature value
    uint16_t raw_humidity;  // Raw humidity value
    uint32_t last_reading;  // Last measurement timestamp
    uint8_t is_available;   // Sensor availability flag
    uint32_t readings_count;
    float min_temp;
    float max_temp;
    float min_humidity;
    float max_humidity;
    uint8_t auto_monitoring;
    uint16_t measurement_interval; // in milliseconds
} hdc1080_sensor_t;

/* Exported constants --------------------------------------------------------*/

/* HDC1080 I2C Address */
#define HDC1080_I2C_ADDRESS     0x40

/* HDC1080 Register Addresses */
#define HDC1080_REG_TEMP        0x00
#define HDC1080_REG_HUMIDITY    0x01
#define HDC1080_REG_CONFIG      0x02
#define HDC1080_REG_SERIAL_1    0xFB
#define HDC1080_REG_SERIAL_2    0xFC
#define HDC1080_REG_SERIAL_3    0xFD
#define HDC1080_REG_MANUF_ID    0xFE
#define HDC1080_REG_DEVICE_ID   0xFF

/* HDC1080 Configuration Register Bits */
#define HDC1080_CONFIG_RST      0x8000  // Software reset
#define HDC1080_CONFIG_HEAT     0x2000  // Heater enable
#define HDC1080_CONFIG_MODE     0x1000  // Measurement mode (0=separate, 1=both)
#define HDC1080_CONFIG_BTST     0x0800  // Battery status
#define HDC1080_CONFIG_TRES     0x0400  // Temperature resolution (0=14bit, 1=11bit)
#define HDC1080_CONFIG_HRES_MASK 0x0300 // Humidity resolution mask
#define HDC1080_CONFIG_HRES_14   0x0000 // 14-bit humidity resolution
#define HDC1080_CONFIG_HRES_11   0x0100 // 11-bit humidity resolution
#define HDC1080_CONFIG_HRES_8    0x0200 // 8-bit humidity resolution

/* HDC1080 Device and Manufacturer IDs */
#define HDC1080_MANUFACTURER_ID 0x5449
#define HDC1080_DEVICE_ID       0x1050

/* HDC1080 Measurement Delays */
#define HDC1080_TEMP_CONV_TIME  7   // Temperature conversion time in ms
#define HDC1080_HUMI_CONV_TIME  7   // Humidity conversion time in ms
#define HDC1080_BOTH_CONV_TIME  15  // Both measurements time in ms

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Initialization and Configuration */
void HDC1080_Init(void);
uint8_t HDC1080_IsAvailable(void);
void HDC1080_Reset(void);
void HDC1080_SetConfig(uint16_t config);
uint16_t HDC1080_GetConfig(void);

/* Measurement Functions */
void HDC1080_StartMeasurement(void);
void HDC1080_ReadBoth(void);
float HDC1080_ReadTemperature(void);
float HDC1080_ReadHumidity(void);
void HDC1080_Update(void);

/* Data Access */
float HDC1080_GetTemperature(void);
float HDC1080_GetHumidity(void);
hdc1080_sensor_t* HDC1080_GetSensorData(void);

/* Utility Functions */
void HDC1080_PrintReading(void);
void HDC1080_PrintStatus(void);
void HDC1080_SetAutoMonitoring(uint8_t enable);
void HDC1080_SetMeasurementInterval(uint16_t interval_ms);

/* Device Information */
uint16_t HDC1080_GetManufacturerID(void);
uint16_t HDC1080_GetDeviceID(void);
void HDC1080_GetSerialNumber(uint8_t *serial);

/* Shell Commands */
shell_status_t cmd_hdc1080_read(int argc, char **argv);
shell_status_t cmd_hdc1080_status(int argc, char **argv);
shell_status_t cmd_hdc1080_config(int argc, char **argv);
shell_status_t cmd_hdc1080_monitor(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* __HDC1080_H */
