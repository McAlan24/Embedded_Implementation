/**
  ******************************************************************************
  * @file           : adxl345.h
  * @brief          : Header for ADXL345 accelerometer sensor functions
  ******************************************************************************
  * @attention
  *
  * ADXL345 3-Axis Accelerometer Sensor Library for STM32H563ZI
  * Supports acceleration measurements and motion detection via I2C
  *
  ******************************************************************************
  */

#ifndef __ADXL345_H
#define __ADXL345_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "shell_terminal.h"

/* Exported types ------------------------------------------------------------*/

/* ADXL345 Acceleration Data */
typedef struct {
    float x;    // X-axis acceleration in g
    float y;    // Y-axis acceleration in g
    float z;    // Z-axis acceleration in g
} adxl345_accel_t;

/* ADXL345 Sensor Configuration */
typedef struct {
    adxl345_accel_t accel;      // Current acceleration values
    int16_t raw_x;              // Raw X-axis value
    int16_t raw_y;              // Raw Y-axis value
    int16_t raw_z;              // Raw Z-axis value
    uint32_t last_reading;      // Last measurement timestamp
    uint8_t is_available;       // Sensor availability flag
    uint32_t readings_count;
    
    // Statistics
    adxl345_accel_t min_accel;
    adxl345_accel_t max_accel;
    
    // Configuration
    uint8_t data_rate;          // Data rate setting
    uint8_t range;              // Measurement range
    uint8_t power_mode;         // Power mode
    uint8_t auto_monitoring;    // Auto monitoring flag
    uint16_t measurement_interval; // in milliseconds
    
    // Motion detection
    uint8_t activity_detected;
    uint8_t inactivity_detected;
    uint8_t free_fall_detected;
    uint8_t tap_detected;
    uint8_t double_tap_detected;
    
    // Thresholds
    float activity_threshold;
    float inactivity_threshold;
    float free_fall_threshold;
    float tap_threshold;
} adxl345_sensor_t;

/* Exported constants --------------------------------------------------------*/

/* ADXL345 I2C Address */
#define ADXL345_I2C_ADDRESS     0x53

/* ADXL345 Register Addresses */
#define ADXL345_REG_DEVID       0x00
#define ADXL345_REG_THRESH_TAP  0x1D
#define ADXL345_REG_OFSX        0x1E
#define ADXL345_REG_OFSY        0x1F
#define ADXL345_REG_OFSZ        0x20
#define ADXL345_REG_DUR         0x21
#define ADXL345_REG_LATENT      0x22
#define ADXL345_REG_WINDOW      0x23
#define ADXL345_REG_THRESH_ACT  0x24
#define ADXL345_REG_THRESH_INACT 0x25
#define ADXL345_REG_TIME_INACT  0x26
#define ADXL345_REG_ACT_INACT_CTL 0x27
#define ADXL345_REG_THRESH_FF   0x28
#define ADXL345_REG_TIME_FF     0x29
#define ADXL345_REG_TAP_AXES    0x2A
#define ADXL345_REG_ACT_TAP_STATUS 0x2B
#define ADXL345_REG_BW_RATE     0x2C
#define ADXL345_REG_POWER_CTL   0x2D
#define ADXL345_REG_INT_ENABLE  0x2E
#define ADXL345_REG_INT_MAP     0x2F
#define ADXL345_REG_INT_SOURCE  0x30
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0      0x32
#define ADXL345_REG_DATAX1      0x33
#define ADXL345_REG_DATAY0      0x34
#define ADXL345_REG_DATAY1      0x35
#define ADXL345_REG_DATAZ0      0x36
#define ADXL345_REG_DATAZ1      0x37
#define ADXL345_REG_FIFO_CTL    0x38
#define ADXL345_REG_FIFO_STATUS 0x39

/* ADXL345 Device ID */
#define ADXL345_DEVICE_ID       0xE5

/* Power Control Register Bits */
#define ADXL345_POWER_CTL_LINK      0x20
#define ADXL345_POWER_CTL_AUTO_SLEEP 0x10
#define ADXL345_POWER_CTL_MEASURE   0x08
#define ADXL345_POWER_CTL_SLEEP     0x04
#define ADXL345_POWER_CTL_WAKEUP_8HZ 0x00
#define ADXL345_POWER_CTL_WAKEUP_4HZ 0x01
#define ADXL345_POWER_CTL_WAKEUP_2HZ 0x02
#define ADXL345_POWER_CTL_WAKEUP_1HZ 0x03

/* Data Format Register Bits */
#define ADXL345_DATA_FORMAT_SELF_TEST 0x80
#define ADXL345_DATA_FORMAT_SPI       0x40
#define ADXL345_DATA_FORMAT_INT_INVERT 0x20
#define ADXL345_DATA_FORMAT_FULL_RES  0x08
#define ADXL345_DATA_FORMAT_JUSTIFY   0x04
#define ADXL345_DATA_FORMAT_RANGE_2G  0x00
#define ADXL345_DATA_FORMAT_RANGE_4G  0x01
#define ADXL345_DATA_FORMAT_RANGE_8G  0x02
#define ADXL345_DATA_FORMAT_RANGE_16G 0x03

/* Bandwidth Rate Register Values */
#define ADXL345_BW_RATE_3200HZ  0x0F
#define ADXL345_BW_RATE_1600HZ  0x0E
#define ADXL345_BW_RATE_800HZ   0x0D
#define ADXL345_BW_RATE_400HZ   0x0C
#define ADXL345_BW_RATE_200HZ   0x0B
#define ADXL345_BW_RATE_100HZ   0x0A
#define ADXL345_BW_RATE_50HZ    0x09
#define ADXL345_BW_RATE_25HZ    0x08
#define ADXL345_BW_RATE_12_5HZ  0x07
#define ADXL345_BW_RATE_6_25HZ  0x06

/* Interrupt Enable/Map/Source Register Bits */
#define ADXL345_INT_DATA_READY  0x80
#define ADXL345_INT_SINGLE_TAP  0x40
#define ADXL345_INT_DOUBLE_TAP  0x20
#define ADXL345_INT_ACTIVITY    0x10
#define ADXL345_INT_INACTIVITY  0x08
#define ADXL345_INT_FREE_FALL   0x04
#define ADXL345_INT_WATERMARK   0x02
#define ADXL345_INT_OVERRUN     0x01

/* Activity/Inactivity Control Register Bits */
#define ADXL345_ACT_INACT_CTL_ACT_AC_DC   0x80
#define ADXL345_ACT_INACT_CTL_ACT_X_EN    0x40
#define ADXL345_ACT_INACT_CTL_ACT_Y_EN    0x20
#define ADXL345_ACT_INACT_CTL_ACT_Z_EN    0x10
#define ADXL345_ACT_INACT_CTL_INACT_AC_DC 0x08
#define ADXL345_ACT_INACT_CTL_INACT_X_EN  0x04
#define ADXL345_ACT_INACT_CTL_INACT_Y_EN  0x02
#define ADXL345_ACT_INACT_CTL_INACT_Z_EN  0x01

/* Tap Axes Register Bits */
#define ADXL345_TAP_AXES_SUPPRESS   0x08
#define ADXL345_TAP_AXES_TAP_X_EN   0x04
#define ADXL345_TAP_AXES_TAP_Y_EN   0x02
#define ADXL345_TAP_AXES_TAP_Z_EN   0x01

/* Scale factors for different ranges */
#define ADXL345_SCALE_2G    0.0039f  // 3.9 mg/LSB
#define ADXL345_SCALE_4G    0.0078f  // 7.8 mg/LSB
#define ADXL345_SCALE_8G    0.0156f  // 15.6 mg/LSB
#define ADXL345_SCALE_16G   0.0312f  // 31.2 mg/LSB

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Initialization and Configuration */
void ADXL345_Init(void);
uint8_t ADXL345_IsAvailable(void);
void ADXL345_SetPowerMode(uint8_t power_mode);
void ADXL345_SetDataRate(uint8_t data_rate);
void ADXL345_SetRange(uint8_t range);
void ADXL345_SetDataFormat(uint8_t format);
void ADXL345_Calibrate(void);

/* Measurement Functions */
void ADXL345_ReadAcceleration(void);
void ADXL345_ReadRawData(int16_t *x, int16_t *y, int16_t *z);
void ADXL345_Update(void);

/* Data Access */
adxl345_accel_t ADXL345_GetAcceleration(void);
adxl345_sensor_t* ADXL345_GetSensorData(void);
float ADXL345_GetMagnitude(void);

/* Motion Detection */
void ADXL345_ConfigureActivityDetection(float threshold, uint8_t axes);
void ADXL345_ConfigureInactivityDetection(float threshold, uint8_t time, uint8_t axes);
void ADXL345_ConfigureFreeFallDetection(float threshold, uint8_t time);
void ADXL345_ConfigureTapDetection(float threshold, uint8_t duration, uint8_t axes);
void ADXL345_ConfigureDoubleTapDetection(float threshold, uint8_t duration, 
                                        uint8_t latent, uint8_t window, uint8_t axes);
void ADXL345_EnableInterrupts(uint8_t interrupts);
uint8_t ADXL345_GetInterruptSource(void);
void ADXL345_ClearInterrupts(void);

/* Utility Functions */
void ADXL345_PrintReading(void);
void ADXL345_PrintStatus(void);
void ADXL345_PrintMotionStatus(void);
void ADXL345_SetAutoMonitoring(uint8_t enable);
void ADXL345_SetMeasurementInterval(uint16_t interval_ms);

/* Device Information */
uint8_t ADXL345_GetDeviceID(void);

/* Shell Commands */
shell_status_t cmd_adxl345_read(int argc, char **argv);
shell_status_t cmd_adxl345_status(int argc, char **argv);
shell_status_t cmd_adxl345_config(int argc, char **argv);
shell_status_t cmd_adxl345_monitor(int argc, char **argv);
shell_status_t cmd_adxl345_motion(int argc, char **argv);
shell_status_t cmd_adxl345_calibrate(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* __ADXL345_H */
