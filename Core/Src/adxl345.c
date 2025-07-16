/**
  ******************************************************************************
  * @file           : adxl345.c
  * @brief          : ADXL345 accelerometer sensor implementation
  ******************************************************************************
  * @attention
  *
  * ADXL345 3-Axis Accelerometer Sensor Library for STM32H563ZI
  * Supports acceleration measurements and motion detection via I2C
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adxl345.h"
#include "display_module.h"
#include "led_timer.h"
#include <math.h>

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
adxl345_sensor_t adxl345_sensor;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ADXL345_WriteRegister(uint8_t reg, uint8_t data);
static HAL_StatusTypeDef ADXL345_ReadRegister(uint8_t reg, uint8_t *data);
static HAL_StatusTypeDef ADXL345_ReadMultipleRegisters(uint8_t reg, uint8_t *data, uint16_t size);
static float ADXL345_ConvertToG(int16_t raw_value);

/* Private user code ---------------------------------------------------------*/

/**
 * @brief Write data to ADXL345 register
 */
static HAL_StatusTypeDef ADXL345_WriteRegister(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2] = {reg, data};
    return HAL_I2C_Master_Transmit(&hi2c1, ADXL345_I2C_ADDRESS << 1, buffer, 2, 1000);
}

/**
 * @brief Read data from ADXL345 register
 */
static HAL_StatusTypeDef ADXL345_ReadRegister(uint8_t reg, uint8_t *data)
{
    HAL_StatusTypeDef status;
    
    // Write register address
    status = HAL_I2C_Master_Transmit(&hi2c1, ADXL345_I2C_ADDRESS << 1, &reg, 1, 1000);
    if (status != HAL_OK) return status;
    
    // Read data
    return HAL_I2C_Master_Receive(&hi2c1, ADXL345_I2C_ADDRESS << 1, data, 1, 1000);
}

/**
 * @brief Read multiple registers from ADXL345
 */
static HAL_StatusTypeDef ADXL345_ReadMultipleRegisters(uint8_t reg, uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    
    // Write register address
    status = HAL_I2C_Master_Transmit(&hi2c1, ADXL345_I2C_ADDRESS << 1, &reg, 1, 1000);
    if (status != HAL_OK) return status;
    
    // Read data
    return HAL_I2C_Master_Receive(&hi2c1, ADXL345_I2C_ADDRESS << 1, data, size, 1000);
}

/**
 * @brief Convert raw acceleration value to g
 */
static float ADXL345_ConvertToG(int16_t raw_value)
{
    float scale_factor;
    
    switch (adxl345_sensor.range) {
        case ADXL345_DATA_FORMAT_RANGE_2G:
            scale_factor = ADXL345_SCALE_2G;
            break;
        case ADXL345_DATA_FORMAT_RANGE_4G:
            scale_factor = ADXL345_SCALE_4G;
            break;
        case ADXL345_DATA_FORMAT_RANGE_8G:
            scale_factor = ADXL345_SCALE_8G;
            break;
        case ADXL345_DATA_FORMAT_RANGE_16G:
            scale_factor = ADXL345_SCALE_16G;
            break;
        default:
            scale_factor = ADXL345_SCALE_2G;
            break;
    }
    
    return (float)raw_value * scale_factor;
}

/**
 * @brief Initialize ADXL345 sensor
 */
void ADXL345_Init(void)
{
    memset(&adxl345_sensor, 0, sizeof(adxl345_sensor_t));
    
    // Initialize min/max values
    adxl345_sensor.min_accel.x = 1000.0f;
    adxl345_sensor.min_accel.y = 1000.0f;
    adxl345_sensor.min_accel.z = 1000.0f;
    adxl345_sensor.max_accel.x = -1000.0f;
    adxl345_sensor.max_accel.y = -1000.0f;
    adxl345_sensor.max_accel.z = -1000.0f;
    
    // Set default configuration
    adxl345_sensor.data_rate = ADXL345_BW_RATE_100HZ;
    adxl345_sensor.range = ADXL345_DATA_FORMAT_RANGE_2G;
    adxl345_sensor.power_mode = ADXL345_POWER_CTL_MEASURE;
    adxl345_sensor.measurement_interval = 100; // Default 100ms
    
    // Set default thresholds
    adxl345_sensor.activity_threshold = 0.5f;    // 0.5g
    adxl345_sensor.inactivity_threshold = 0.1f;  // 0.1g
    adxl345_sensor.free_fall_threshold = 0.3f;   // 0.3g
    adxl345_sensor.tap_threshold = 3.0f;         // 3.0g
    
    // Check if sensor is available
    if (ADXL345_IsAvailable()) {
        // Configure data format: full resolution, ±2g range
        ADXL345_SetDataFormat(ADXL345_DATA_FORMAT_FULL_RES | ADXL345_DATA_FORMAT_RANGE_2G);
        
        // Set data rate to 100Hz
        ADXL345_SetDataRate(ADXL345_BW_RATE_100HZ);
        
        // Enable measurement mode
        ADXL345_SetPowerMode(ADXL345_POWER_CTL_MEASURE);
        
        // Small delay for sensor to stabilize
        HAL_Delay(10);
        
        // Perform initial reading
        ADXL345_Update();
        
        adxl345_sensor.is_available = 1;
        Shell_PrintColored(COLOR_GREEN, "ADXL345 3-Axis Accelerometer initialized\r\n");
    } else {
        adxl345_sensor.is_available = 0;
        Shell_PrintColored(COLOR_RED, "ADXL345 Sensor not found!\r\n");
    }
}

/**
 * @brief Check if ADXL345 sensor is available
 */
uint8_t ADXL345_IsAvailable(void)
{
    uint8_t device_id;
    
    if (ADXL345_ReadRegister(ADXL345_REG_DEVID, &device_id) != HAL_OK) {
        return 0;
    }
    
    return (device_id == ADXL345_DEVICE_ID);
}

/**
 * @brief Set ADXL345 power mode
 */
void ADXL345_SetPowerMode(uint8_t power_mode)
{
    adxl345_sensor.power_mode = power_mode;
    ADXL345_WriteRegister(ADXL345_REG_POWER_CTL, power_mode);
}

/**
 * @brief Set ADXL345 data rate
 */
void ADXL345_SetDataRate(uint8_t data_rate)
{
    adxl345_sensor.data_rate = data_rate;
    ADXL345_WriteRegister(ADXL345_REG_BW_RATE, data_rate);
}

/**
 * @brief Set ADXL345 measurement range
 */
void ADXL345_SetRange(uint8_t range)
{
    adxl345_sensor.range = range;
    uint8_t data_format = ADXL345_DATA_FORMAT_FULL_RES | range;
    ADXL345_WriteRegister(ADXL345_REG_DATA_FORMAT, data_format);
}

/**
 * @brief Set ADXL345 data format
 */
void ADXL345_SetDataFormat(uint8_t format)
{
    adxl345_sensor.range = format & 0x03;
    ADXL345_WriteRegister(ADXL345_REG_DATA_FORMAT, format);
}

/**
 * @brief Calibrate ADXL345 sensor (should be done on level surface)
 */
void ADXL345_Calibrate(void)
{
    if (!adxl345_sensor.is_available) return;
    
    Shell_PrintColored(COLOR_YELLOW, "Calibrating ADXL345... Place on level surface\r\n");
    
    // Take multiple readings and average
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    const int samples = 100;
    
    for (int i = 0; i < samples; i++) {
        int16_t raw_x, raw_y, raw_z;
        ADXL345_ReadRawData(&raw_x, &raw_y, &raw_z);
        sum_x += raw_x;
        sum_y += raw_y;
        sum_z += raw_z;
        HAL_Delay(10);
    }
    
    // Calculate offset values
    int16_t offset_x = -(sum_x / samples);
    int16_t offset_y = -(sum_y / samples);
    int16_t offset_z = -(sum_z / samples) + (int16_t)(1.0f / ADXL345_SCALE_2G); // 1g for Z-axis
    
    // Write offset values (scale to register format: 15.6mg/LSB)
    ADXL345_WriteRegister(ADXL345_REG_OFSX, (uint8_t)(offset_x / 4));
    ADXL345_WriteRegister(ADXL345_REG_OFSY, (uint8_t)(offset_y / 4));
    ADXL345_WriteRegister(ADXL345_REG_OFSZ, (uint8_t)(offset_z / 4));
    
    Shell_PrintColored(COLOR_GREEN, "ADXL345 calibration complete\r\n");
    Shell_Printf("Offsets: X=%d, Y=%d, Z=%d\r\n", offset_x/4, offset_y/4, offset_z/4);
}

/**
 * @brief Read acceleration data from ADXL345
 */
void ADXL345_ReadAcceleration(void)
{
    if (!adxl345_sensor.is_available) return;
    
    uint8_t data[6];
    
    // Read all 6 acceleration registers (X, Y, Z - 2 bytes each)
    if (ADXL345_ReadMultipleRegisters(ADXL345_REG_DATAX0, data, 6) == HAL_OK) {
        // Combine bytes to form 16-bit values
        adxl345_sensor.raw_x = (int16_t)((data[1] << 8) | data[0]);
        adxl345_sensor.raw_y = (int16_t)((data[3] << 8) | data[2]);
        adxl345_sensor.raw_z = (int16_t)((data[5] << 8) | data[4]);
        
        // Convert to g units
        adxl345_sensor.accel.x = ADXL345_ConvertToG(adxl345_sensor.raw_x);
        adxl345_sensor.accel.y = ADXL345_ConvertToG(adxl345_sensor.raw_y);
        adxl345_sensor.accel.z = ADXL345_ConvertToG(adxl345_sensor.raw_z);
        
        // Update statistics
        adxl345_sensor.readings_count++;
        adxl345_sensor.last_reading = HAL_GetTick();
        
        // Update min/max values
        if (adxl345_sensor.accel.x < adxl345_sensor.min_accel.x) {
            adxl345_sensor.min_accel.x = adxl345_sensor.accel.x;
        }
        if (adxl345_sensor.accel.x > adxl345_sensor.max_accel.x) {
            adxl345_sensor.max_accel.x = adxl345_sensor.accel.x;
        }
        
        if (adxl345_sensor.accel.y < adxl345_sensor.min_accel.y) {
            adxl345_sensor.min_accel.y = adxl345_sensor.accel.y;
        }
        if (adxl345_sensor.accel.y > adxl345_sensor.max_accel.y) {
            adxl345_sensor.max_accel.y = adxl345_sensor.accel.y;
        }
        
        if (adxl345_sensor.accel.z < adxl345_sensor.min_accel.z) {
            adxl345_sensor.min_accel.z = adxl345_sensor.accel.z;
        }
        if (adxl345_sensor.accel.z > adxl345_sensor.max_accel.z) {
            adxl345_sensor.max_accel.z = adxl345_sensor.accel.z;
        }
    }
}

/**
 * @brief Read raw acceleration data
 */
void ADXL345_ReadRawData(int16_t *x, int16_t *y, int16_t *z)
{
    if (!adxl345_sensor.is_available) {
        *x = *y = *z = 0;
        return;
    }
    
    uint8_t data[6];
    
    if (ADXL345_ReadMultipleRegisters(ADXL345_REG_DATAX0, data, 6) == HAL_OK) {
        *x = (int16_t)((data[1] << 8) | data[0]);
        *y = (int16_t)((data[3] << 8) | data[2]);
        *z = (int16_t)((data[5] << 8) | data[4]);
    } else {
        *x = *y = *z = 0;
    }
}

/**
 * @brief Update sensor readings
 */
void ADXL345_Update(void)
{
    ADXL345_ReadAcceleration();
}

/**
 * @brief Get current acceleration values
 */
adxl345_accel_t ADXL345_GetAcceleration(void)
{
    return adxl345_sensor.accel;
}

/**
 * @brief Get sensor data structure
 */
adxl345_sensor_t* ADXL345_GetSensorData(void)
{
    return &adxl345_sensor;
}

/**
 * @brief Get acceleration magnitude
 */
float ADXL345_GetMagnitude(void)
{
    return sqrtf(adxl345_sensor.accel.x * adxl345_sensor.accel.x +
                 adxl345_sensor.accel.y * adxl345_sensor.accel.y +
                 adxl345_sensor.accel.z * adxl345_sensor.accel.z);
}

/**
 * @brief Configure activity detection
 */
void ADXL345_ConfigureActivityDetection(float threshold, uint8_t axes)
{
    // Convert threshold to register format (62.5mg/LSB)
    uint8_t thresh_act = (uint8_t)(threshold / 0.0625f);
    ADXL345_WriteRegister(ADXL345_REG_THRESH_ACT, thresh_act);
    
    // Configure axes and AC/DC coupling
    uint8_t act_inact_ctl = axes << 4; // Activity axes in upper nibble
    ADXL345_WriteRegister(ADXL345_REG_ACT_INACT_CTL, act_inact_ctl);
    
    adxl345_sensor.activity_threshold = threshold;
}

/**
 * @brief Configure inactivity detection
 */
void ADXL345_ConfigureInactivityDetection(float threshold, uint8_t time, uint8_t axes)
{
    // Convert threshold to register format (62.5mg/LSB)
    uint8_t thresh_inact = (uint8_t)(threshold / 0.0625f);
    ADXL345_WriteRegister(ADXL345_REG_THRESH_INACT, thresh_inact);
    
    // Set inactivity time (1 second/LSB)
    ADXL345_WriteRegister(ADXL345_REG_TIME_INACT, time);
    
    // Configure axes
    uint8_t act_inact_ctl;
    ADXL345_ReadRegister(ADXL345_REG_ACT_INACT_CTL, &act_inact_ctl);
    act_inact_ctl |= axes; // Inactivity axes in lower nibble
    ADXL345_WriteRegister(ADXL345_REG_ACT_INACT_CTL, act_inact_ctl);
    
    adxl345_sensor.inactivity_threshold = threshold;
}

/**
 * @brief Configure free fall detection
 */
void ADXL345_ConfigureFreeFallDetection(float threshold, uint8_t time)
{
    // Convert threshold to register format (62.5mg/LSB)
    uint8_t thresh_ff = (uint8_t)(threshold / 0.0625f);
    ADXL345_WriteRegister(ADXL345_REG_THRESH_FF, thresh_ff);
    
    // Set free fall time (5ms/LSB)
    ADXL345_WriteRegister(ADXL345_REG_TIME_FF, time);
    
    adxl345_sensor.free_fall_threshold = threshold;
}

/**
 * @brief Configure tap detection
 */
void ADXL345_ConfigureTapDetection(float threshold, uint8_t duration, uint8_t axes)
{
    // Convert threshold to register format (62.5mg/LSB)
    uint8_t thresh_tap = (uint8_t)(threshold / 0.0625f);
    ADXL345_WriteRegister(ADXL345_REG_THRESH_TAP, thresh_tap);
    
    // Set tap duration (625μs/LSB)
    ADXL345_WriteRegister(ADXL345_REG_DUR, duration);
    
    // Configure tap axes
    ADXL345_WriteRegister(ADXL345_REG_TAP_AXES, axes);
    
    adxl345_sensor.tap_threshold = threshold;
}

/**
 * @brief Configure double tap detection
 */
void ADXL345_ConfigureDoubleTapDetection(float threshold, uint8_t duration,
                                        uint8_t latent, uint8_t window, uint8_t axes)
{
    // Configure single tap first
    ADXL345_ConfigureTapDetection(threshold, duration, axes);
    
    // Set latent time (1.25ms/LSB)
    ADXL345_WriteRegister(ADXL345_REG_LATENT, latent);
    
    // Set window time (1.25ms/LSB)
    ADXL345_WriteRegister(ADXL345_REG_WINDOW, window);
}

/**
 * @brief Enable interrupts
 */
void ADXL345_EnableInterrupts(uint8_t interrupts)
{
    ADXL345_WriteRegister(ADXL345_REG_INT_ENABLE, interrupts);
}

/**
 * @brief Get interrupt source
 */
uint8_t ADXL345_GetInterruptSource(void)
{
    uint8_t int_source;
    ADXL345_ReadRegister(ADXL345_REG_INT_SOURCE, &int_source);
    return int_source;
}

/**
 * @brief Clear interrupts by reading interrupt source
 */
void ADXL345_ClearInterrupts(void)
{
    ADXL345_GetInterruptSource();
}

/**
 * @brief Print current sensor reading
 */
void ADXL345_PrintReading(void)
{

    
    ADXL345_Update();
    
    Shell_PrintColored(COLOR_CYAN, "ADXL345 Accelerometer Reading:\r\n");
    Shell_Printf("  X-axis: %+.3f g (raw: %d)\r\n", adxl345_sensor.accel.x, adxl345_sensor.raw_x);
    Shell_Printf("  Y-axis: %+.3f g (raw: %d)\r\n", adxl345_sensor.accel.y, adxl345_sensor.raw_y);
    Shell_Printf("  Z-axis: %+.3f g (raw: %d)\r\n", adxl345_sensor.accel.z, adxl345_sensor.raw_z);
    Shell_Printf("  Magnitude: %.3f g\r\n", ADXL345_GetMagnitude());
    Shell_Printf("  Last Reading: %lu ms ago\r\n", HAL_GetTick() - adxl345_sensor.last_reading);
}

/**
 * @brief Print sensor status and statistics
 */
void ADXL345_PrintStatus(void)
{
  
    
    Shell_PrintColored(COLOR_CYAN, "ADXL345 Sensor Status:\r\n");
    Shell_Printf("  Device ID: 0x%02X\r\n", ADXL345_GetDeviceID());
    Shell_Printf("  Data Rate: 0x%02X\r\n", adxl345_sensor.data_rate);
    Shell_Printf("  Range: ±%dg\r\n", (adxl345_sensor.range == 0) ? 2 : 
                (adxl345_sensor.range == 1) ? 4 : 
                (adxl345_sensor.range == 2) ? 8 : 16);
    Shell_Printf("  Total Readings: %lu\r\n", adxl345_sensor.readings_count);
    Shell_Printf("  X Range: %+.3f g to %+.3f g\r\n", 
                adxl345_sensor.min_accel.x, adxl345_sensor.max_accel.x);
    Shell_Printf("  Y Range: %+.3f g to %+.3f g\r\n", 
                adxl345_sensor.min_accel.y, adxl345_sensor.max_accel.y);
    Shell_Printf("  Z Range: %+.3f g to %+.3f g\r\n", 
                adxl345_sensor.min_accel.z, adxl345_sensor.max_accel.z);
    Shell_Printf("  Auto Monitoring: %s\r\n", adxl345_sensor.auto_monitoring ? "ON" : "OFF");
    
    if (adxl345_sensor.auto_monitoring) {
        Shell_Printf("  Measurement Interval: %u ms\r\n", adxl345_sensor.measurement_interval);
    }
}

/**
 * @brief Print motion detection status
 */
void ADXL345_PrintMotionStatus(void)
{

    
    uint8_t int_source = ADXL345_GetInterruptSource();
    
    Shell_PrintColored(COLOR_CYAN, "ADXL345 Motion Detection Status:\r\n");
    Shell_Printf("  Activity Threshold: %.3f g\r\n", adxl345_sensor.activity_threshold);
    Shell_Printf("  Inactivity Threshold: %.3f g\r\n", adxl345_sensor.inactivity_threshold);
    Shell_Printf("  Free Fall Threshold: %.3f g\r\n", adxl345_sensor.free_fall_threshold);
    Shell_Printf("  Tap Threshold: %.3f g\r\n", adxl345_sensor.tap_threshold);
    
    Shell_PrintColored(COLOR_YELLOW, "  Current Status:\r\n");
    Shell_Printf("    Activity: %s\r\n", (int_source & ADXL345_INT_ACTIVITY) ? "DETECTED" : "None");
    Shell_Printf("    Inactivity: %s\r\n", (int_source & ADXL345_INT_INACTIVITY) ? "DETECTED" : "None");
    Shell_Printf("    Free Fall: %s\r\n", (int_source & ADXL345_INT_FREE_FALL) ? "DETECTED" : "None");
    Shell_Printf("    Single Tap: %s\r\n", (int_source & ADXL345_INT_SINGLE_TAP) ? "DETECTED" : "None");
    Shell_Printf("    Double Tap: %s\r\n", (int_source & ADXL345_INT_DOUBLE_TAP) ? "DETECTED" : "None");
}

/**
 * @brief Enable/disable auto monitoring
 */
void ADXL345_SetAutoMonitoring(uint8_t enable)
{
    adxl345_sensor.auto_monitoring = enable;
}

/**
 * @brief Set measurement interval for auto monitoring
 */
void ADXL345_SetMeasurementInterval(uint16_t interval_ms)
{
    adxl345_sensor.measurement_interval = interval_ms;
}

/**
 * @brief Get device ID
 */
uint8_t ADXL345_GetDeviceID(void)
{
    uint8_t id;
    if (ADXL345_ReadRegister(ADXL345_REG_DEVID, &id) == HAL_OK) {
        return id;
    }
    return 0;
}

/* Shell Commands */

/**
 * @brief Shell command to read ADXL345 sensor
 */
shell_status_t cmd_adxl345_read(int argc, char **argv)
{
    ADXL345_PrintReading();
    return SHELL_OK;
}

/**
 * @brief Shell command to show ADXL345 status
 */
shell_status_t cmd_adxl345_status(int argc, char **argv)
{
    ADXL345_PrintStatus();
    return SHELL_OK;
}

/**
 * @brief Shell command to configure ADXL345
 */
shell_status_t cmd_adxl345_config(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: adxl345_config <option> [value]\r\n");
        Shell_Print("Options:\r\n");
        Shell_Print("  range <2|4|8|16>    - Set measurement range in g\r\n");
        Shell_Print("  rate <value>        - Set data rate (0x06-0x0F)\r\n");
        Shell_Print("  format <value>      - Set data format register\r\n");
        Shell_Print("  power <value>       - Set power control register\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "range") == 0 && argc >= 3) {
        uint8_t range_val = (uint8_t)atoi(argv[2]);
        uint8_t range_reg;
        
        switch (range_val) {
            case 2:  range_reg = ADXL345_DATA_FORMAT_RANGE_2G; break;
            case 4:  range_reg = ADXL345_DATA_FORMAT_RANGE_4G; break;
            case 8:  range_reg = ADXL345_DATA_FORMAT_RANGE_8G; break;
            case 16: range_reg = ADXL345_DATA_FORMAT_RANGE_16G; break;
            default:
                Shell_PrintColored(COLOR_RED, "Invalid range. Use 2, 4, 8, or 16\r\n");
                return SHELL_ERROR;
        }
        
        ADXL345_SetRange(range_reg);
        Shell_Printf("Range set to ±%dg\r\n", range_val);
    }
    else if (strcmp(argv[1], "rate") == 0 && argc >= 3) {
        uint8_t rate = (uint8_t)strtol(argv[2], NULL, 16);
        ADXL345_SetDataRate(rate);
        Shell_Printf("Data rate set to 0x%02X\r\n", rate);
    }
    else if (strcmp(argv[1], "format") == 0 && argc >= 3) {
        uint8_t format = (uint8_t)strtol(argv[2], NULL, 16);
        ADXL345_SetDataFormat(format);
        Shell_Printf("Data format set to 0x%02X\r\n", format);
    }
    else if (strcmp(argv[1], "power") == 0 && argc >= 3) {
        uint8_t power = (uint8_t)strtol(argv[2], NULL, 16);
        ADXL345_SetPowerMode(power);
        Shell_Printf("Power mode set to 0x%02X\r\n", power);
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid configuration option\r\n");
    }
    
    return SHELL_OK;
}

/**
 * @brief Shell command to control ADXL345 monitoring
 */
shell_status_t cmd_adxl345_monitor(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: adxl345_monitor <on/off> [interval_ms]\r\n");
        Shell_Print("  on/off      - Enable/disable auto monitoring\r\n");
        Shell_Print("  interval_ms - Measurement interval in milliseconds\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "on") == 0) {
        if (argc >= 3) {
            uint16_t interval = (uint16_t)atoi(argv[2]);
            if (interval >= 10) {
                ADXL345_SetMeasurementInterval(interval);
            }
        }
        ADXL345_SetAutoMonitoring(1);
        Shell_PrintColored(COLOR_GREEN, "ADXL345 auto monitoring enabled\r\n");
        Shell_Printf("Measurement interval: %u ms\r\n", adxl345_sensor.measurement_interval);
    }
    else if (strcmp(argv[1], "off") == 0) {
        ADXL345_SetAutoMonitoring(0);
        Shell_PrintColored(COLOR_YELLOW, "ADXL345 auto monitoring disabled\r\n");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid option. Use 'on' or 'off'\r\n");
    }
    
    return SHELL_OK;
}

/**
 * @brief Shell command to configure motion detection
 */
shell_status_t cmd_adxl345_motion(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: adxl345_motion <option> [parameters]\r\n");
        Shell_Print("Options:\r\n");
        Shell_Print("  status              - Show motion detection status\r\n");
        Shell_Print("  activity <thresh>   - Configure activity detection (g)\r\n");
        Shell_Print("  inactivity <thresh> <time> - Configure inactivity (g, seconds)\r\n");
        Shell_Print("  freefall <thresh> <time>   - Configure free fall (g, 5ms units)\r\n");
        Shell_Print("  tap <thresh>        - Configure tap detection (g)\r\n");
        Shell_Print("  enable <interrupts> - Enable interrupts (hex mask)\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "status") == 0) {
        ADXL345_PrintMotionStatus();
    }
    else if (strcmp(argv[1], "activity") == 0 && argc >= 3) {
        float threshold = atof(argv[2]);
        ADXL345_ConfigureActivityDetection(threshold, 0x70); // All axes
        Shell_Printf("Activity detection set to %.3f g\r\n", threshold);
    }
    else if (strcmp(argv[1], "inactivity") == 0 && argc >= 4) {
        float threshold = atof(argv[2]);
        uint8_t time = (uint8_t)atoi(argv[3]);
        ADXL345_ConfigureInactivityDetection(threshold, time, 0x07); // All axes
        Shell_Printf("Inactivity detection set to %.3f g for %d seconds\r\n", threshold, time);
    }
    else if (strcmp(argv[1], "freefall") == 0 && argc >= 4) {
        float threshold = atof(argv[2]);
        uint8_t time = (uint8_t)atoi(argv[3]);
        ADXL345_ConfigureFreeFallDetection(threshold, time);
        Shell_Printf("Free fall detection set to %.3f g for %d*5ms\r\n", threshold, time);
    }
    else if (strcmp(argv[1], "tap") == 0 && argc >= 3) {
        float threshold = atof(argv[2]);
        ADXL345_ConfigureTapDetection(threshold, 0x50, 0x07); // All axes
        Shell_Printf("Tap detection set to %.3f g\r\n", threshold);
    }
    else if (strcmp(argv[1], "enable") == 0 && argc >= 3) {
        uint8_t interrupts = (uint8_t)strtol(argv[2], NULL, 16);
        ADXL345_EnableInterrupts(interrupts);
        Shell_Printf("Interrupts enabled: 0x%02X\r\n", interrupts);
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid motion detection option\r\n");
    }
    
    return SHELL_OK;
}

/**
 * @brief Shell command to calibrate ADXL345
 */
shell_status_t cmd_adxl345_calibrate(int argc, char **argv)
{
    ADXL345_Calibrate();
    return SHELL_OK;
}
