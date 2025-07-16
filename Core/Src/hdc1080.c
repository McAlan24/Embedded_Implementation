/**
  ******************************************************************************
  * @file           : hdc1080.c
  * @brief          : HDC1080 temperature/humidity sensor implementation
  ******************************************************************************
  * @attention
  *
  * HDC1080 Temperature/Humidity Sensor Library for STM32H563ZI
  * Supports temperature and humidity measurements via I2C
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hdc1080.h"
#include "display_module.h"
#include "led_timer.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
hdc1080_sensor_t hdc1080_sensor;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef HDC1080_WriteRegister(uint8_t reg, uint16_t data);
static HAL_StatusTypeDef HDC1080_ReadRegister(uint8_t reg, uint16_t *data);

/* Private user code ---------------------------------------------------------*/

/**
 * @brief Write data to HDC1080 register
 */
static HAL_StatusTypeDef HDC1080_WriteRegister(uint8_t reg, uint16_t data)
{
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = (data >> 8) & 0xFF;  // MSB
    buffer[2] = data & 0xFF;         // LSB
    
    return HAL_I2C_Master_Transmit(&hi2c1, HDC1080_I2C_ADDRESS << 1, buffer, 3, 1000);
}

/**
 * @brief Read data from HDC1080 register
 */
static HAL_StatusTypeDef HDC1080_ReadRegister(uint8_t reg, uint16_t *data)
{
    uint8_t buffer[2];
    HAL_StatusTypeDef status;
    
    // Write register address
    status = HAL_I2C_Master_Transmit(&hi2c1, HDC1080_I2C_ADDRESS << 1, &reg, 1, 1000);
    if (status != HAL_OK) return status;
    
    // Read data
    status = HAL_I2C_Master_Receive(&hi2c1, HDC1080_I2C_ADDRESS << 1, buffer, 2, 1000);
    if (status == HAL_OK) {
        *data = ((uint16_t)buffer[0] << 8) | buffer[1];
    }
    
    return status;
}

/**
 * @brief Initialize HDC1080 sensor
 */
void HDC1080_Init(void)
{
    memset(&hdc1080_sensor, 0, sizeof(hdc1080_sensor_t));
    
    // Initialize min/max values
    hdc1080_sensor.min_temp = 1000.0f;
    hdc1080_sensor.max_temp = -1000.0f;
    hdc1080_sensor.min_humidity = 1000.0f;
    hdc1080_sensor.max_humidity = -1000.0f;
    hdc1080_sensor.measurement_interval = 1000; // Default 1 second
    
    // Reset sensor
    HDC1080_Reset();
    HAL_Delay(20);
    
    // Check if sensor is available
    if (HDC1080_IsAvailable()) {
        // Configure sensor for both temperature and humidity measurement
        // 14-bit resolution for both, heater disabled
        HDC1080_SetConfig(HDC1080_CONFIG_MODE | HDC1080_CONFIG_HRES_14);
        
        // Perform initial reading
        HDC1080_Update();
        
        hdc1080_sensor.is_available = 1;
        Shell_PrintColored(COLOR_GREEN, "HDC1080 Temperature/Humidity Sensor initialized\r\n");
    } else {
        hdc1080_sensor.is_available = 0;
        Shell_PrintColored(COLOR_RED, "HDC1080 Sensor not found!\r\n");
    }
}

/**
 * @brief Check if HDC1080 sensor is available
 */
uint8_t HDC1080_IsAvailable(void)
{
    uint16_t device_id;
    uint16_t manufacturer_id;
    
    if (HDC1080_ReadRegister(HDC1080_REG_DEVICE_ID, &device_id) != HAL_OK) {
        return 0;
    }
    
    if (HDC1080_ReadRegister(HDC1080_REG_MANUF_ID, &manufacturer_id) != HAL_OK) {
        return 0;
    }
    
    return (device_id == HDC1080_DEVICE_ID && manufacturer_id == HDC1080_MANUFACTURER_ID);
}

/**
 * @brief Reset HDC1080 sensor
 */
void HDC1080_Reset(void)
{
    HDC1080_WriteRegister(HDC1080_REG_CONFIG, HDC1080_CONFIG_RST);
}

/**
 * @brief Set HDC1080 configuration
 */
void HDC1080_SetConfig(uint16_t config)
{
    HDC1080_WriteRegister(HDC1080_REG_CONFIG, config);
}

/**
 * @brief Get HDC1080 configuration
 */
uint16_t HDC1080_GetConfig(void)
{
    uint16_t config;
    if (HDC1080_ReadRegister(HDC1080_REG_CONFIG, &config) == HAL_OK) {
        return config;
    }
    return 0;
}

/**
 * @brief Start measurement (trigger conversion)
 */
void HDC1080_StartMeasurement(void)
{
    uint8_t reg = HDC1080_REG_TEMP;
    HAL_I2C_Master_Transmit(&hi2c1, HDC1080_I2C_ADDRESS << 1, &reg, 1, 1000);
}

/**
 * @brief Read both temperature and humidity
 */
void HDC1080_ReadBoth(void)
{
    if (!hdc1080_sensor.is_available) return;
    
    uint8_t data[4];
    
    // Start measurement
    HDC1080_StartMeasurement();
    
    // Wait for conversion
    HAL_Delay(HDC1080_BOTH_CONV_TIME);
    
    // Read both temperature and humidity data
    if (HAL_I2C_Master_Receive(&hi2c1, HDC1080_I2C_ADDRESS << 1, data, 4, 1000) == HAL_OK) {
        // Process temperature (first 2 bytes)
        hdc1080_sensor.raw_temp = ((uint16_t)data[0] << 8) | data[1];
        hdc1080_sensor.temperature = ((float)hdc1080_sensor.raw_temp / 65536.0f) * 165.0f - 40.0f;
        
        // Process humidity (next 2 bytes)
        hdc1080_sensor.raw_humidity = ((uint16_t)data[2] << 8) | data[3];
        hdc1080_sensor.humidity = ((float)hdc1080_sensor.raw_humidity / 65536.0f) * 100.0f;
        
        // Update statistics
        hdc1080_sensor.readings_count++;
        hdc1080_sensor.last_reading = HAL_GetTick();
        
        // Update min/max values
        if (hdc1080_sensor.temperature < hdc1080_sensor.min_temp) {
            hdc1080_sensor.min_temp = hdc1080_sensor.temperature;
        }
        if (hdc1080_sensor.temperature > hdc1080_sensor.max_temp) {
            hdc1080_sensor.max_temp = hdc1080_sensor.temperature;
        }
        
        if (hdc1080_sensor.humidity < hdc1080_sensor.min_humidity) {
            hdc1080_sensor.min_humidity = hdc1080_sensor.humidity;
        }
        if (hdc1080_sensor.humidity > hdc1080_sensor.max_humidity) {
            hdc1080_sensor.max_humidity = hdc1080_sensor.humidity;
        }
    }
}

/**
 * @brief Read temperature only
 */
float HDC1080_ReadTemperature(void)
{
    if (!hdc1080_sensor.is_available) return -999.0f;
    
    uint8_t reg = HDC1080_REG_TEMP;
    uint8_t data[2];
    
    // Start temperature measurement
    HAL_I2C_Master_Transmit(&hi2c1, HDC1080_I2C_ADDRESS << 1, &reg, 1, 1000);
    
    // Wait for conversion
    HAL_Delay(HDC1080_TEMP_CONV_TIME);
    
    // Read temperature data
    if (HAL_I2C_Master_Receive(&hi2c1, HDC1080_I2C_ADDRESS << 1, data, 2, 1000) == HAL_OK) {
        uint16_t raw_temp = ((uint16_t)data[0] << 8) | data[1];
        return ((float)raw_temp / 65536.0f) * 165.0f - 40.0f;
    }
    
    return -999.0f;
}

/**
 * @brief Read humidity only
 */
float HDC1080_ReadHumidity(void)
{
    if (!hdc1080_sensor.is_available) return -999.0f;
    
    uint8_t reg = HDC1080_REG_HUMIDITY;
    uint8_t data[2];
    
    // Start humidity measurement
    HAL_I2C_Master_Transmit(&hi2c1, HDC1080_I2C_ADDRESS << 1, &reg, 1, 1000);
    
    // Wait for conversion
    HAL_Delay(HDC1080_HUMI_CONV_TIME);
    
    // Read humidity data
    if (HAL_I2C_Master_Receive(&hi2c1, HDC1080_I2C_ADDRESS << 1, data, 2, 1000) == HAL_OK) {
        uint16_t raw_humidity = ((uint16_t)data[0] << 8) | data[1];
        return ((float)raw_humidity / 65536.0f) * 100.0f;
    }
    
    return -999.0f;
}

/**
 * @brief Update sensor readings
 */
void HDC1080_Update(void)
{
    HDC1080_ReadBoth();
}

/**
 * @brief Get current temperature
 */
float HDC1080_GetTemperature(void)
{
    return hdc1080_sensor.temperature;
}

/**
 * @brief Get current humidity
 */
float HDC1080_GetHumidity(void)
{
    return hdc1080_sensor.humidity;
}

/**
 * @brief Get sensor data structure
 */
hdc1080_sensor_t* HDC1080_GetSensorData(void)
{
    return &hdc1080_sensor;
}

/**
 * @brief Print current sensor reading
 */
void HDC1080_PrintReading(void)
{
    if (!hdc1080_sensor.is_available) {
        Shell_PrintColored(COLOR_RED, "HDC1080 Sensor not available\r\n");
        return;
    }
    
    HDC1080_Update();
    
    Shell_PrintColored(COLOR_CYAN, "HDC1080 Sensor Reading:\r\n");
    Shell_Printf("  Temperature: %.2f°C\r\n", hdc1080_sensor.temperature);
    Shell_Printf("  Humidity: %.2f%%\r\n", hdc1080_sensor.humidity);
    Shell_Printf("  Raw Temperature: %u\r\n", hdc1080_sensor.raw_temp);
    Shell_Printf("  Raw Humidity: %u\r\n", hdc1080_sensor.raw_humidity);
    Shell_Printf("  Last Reading: %lu ms ago\r\n", HAL_GetTick() - hdc1080_sensor.last_reading);
}

/**
 * @brief Print sensor status and statistics
 */
void HDC1080_PrintStatus(void)
{
    if (!hdc1080_sensor.is_available) {
        Shell_PrintColored(COLOR_RED, "HDC1080 Sensor not available\r\n");
        return;
    }
    
    Shell_PrintColored(COLOR_CYAN, "HDC1080 Sensor Status:\r\n");
    Shell_Printf("  Device ID: 0x%04X\r\n", HDC1080_GetDeviceID());
    Shell_Printf("  Manufacturer ID: 0x%04X\r\n", HDC1080_GetManufacturerID());
    Shell_Printf("  Configuration: 0x%04X\r\n", HDC1080_GetConfig());
    Shell_Printf("  Total Readings: %lu\r\n", hdc1080_sensor.readings_count);
    Shell_Printf("  Temperature Range: %.2f°C to %.2f°C\r\n", 
                hdc1080_sensor.min_temp, hdc1080_sensor.max_temp);
    Shell_Printf("  Humidity Range: %.2f%% to %.2f%%\r\n", 
                hdc1080_sensor.min_humidity, hdc1080_sensor.max_humidity);
    Shell_Printf("  Auto Monitoring: %s\r\n", hdc1080_sensor.auto_monitoring ? "ON" : "OFF");
    
    if (hdc1080_sensor.auto_monitoring) {
        Shell_Printf("  Measurement Interval: %u ms\r\n", hdc1080_sensor.measurement_interval);
    }
}

/**
 * @brief Enable/disable auto monitoring
 */
void HDC1080_SetAutoMonitoring(uint8_t enable)
{
    hdc1080_sensor.auto_monitoring = enable;
}

/**
 * @brief Set measurement interval for auto monitoring
 */
void HDC1080_SetMeasurementInterval(uint16_t interval_ms)
{
    hdc1080_sensor.measurement_interval = interval_ms;
}

/**
 * @brief Get manufacturer ID
 */
uint16_t HDC1080_GetManufacturerID(void)
{
    uint16_t id;
    if (HDC1080_ReadRegister(HDC1080_REG_MANUF_ID, &id) == HAL_OK) {
        return id;
    }
    return 0;
}

/**
 * @brief Get device ID
 */
uint16_t HDC1080_GetDeviceID(void)
{
    uint16_t id;
    if (HDC1080_ReadRegister(HDC1080_REG_DEVICE_ID, &id) == HAL_OK) {
        return id;
    }
    return 0;
}

/**
 * @brief Get serial number
 */
void HDC1080_GetSerialNumber(uint8_t *serial)
{
    uint16_t serial_parts[3];
    
    if (HDC1080_ReadRegister(HDC1080_REG_SERIAL_1, &serial_parts[0]) == HAL_OK &&
        HDC1080_ReadRegister(HDC1080_REG_SERIAL_2, &serial_parts[1]) == HAL_OK &&
        HDC1080_ReadRegister(HDC1080_REG_SERIAL_3, &serial_parts[2]) == HAL_OK) {
        
        serial[0] = (serial_parts[0] >> 8) & 0xFF;
        serial[1] = serial_parts[0] & 0xFF;
        serial[2] = (serial_parts[1] >> 8) & 0xFF;
        serial[3] = serial_parts[1] & 0xFF;
        serial[4] = (serial_parts[2] >> 8) & 0xFF;
        serial[5] = serial_parts[2] & 0xFF;
    } else {
        memset(serial, 0, 6);
    }
}

/* Shell Commands */

/**
 * @brief Shell command to read HDC1080 sensor
 */
shell_status_t cmd_hdc1080_read(int argc, char **argv)
{
    HDC1080_PrintReading();
    return SHELL_OK;
}

/**
 * @brief Shell command to show HDC1080 status
 */
shell_status_t cmd_hdc1080_status(int argc, char **argv)
{
    HDC1080_PrintStatus();
    return SHELL_OK;
}

/**
 * @brief Shell command to configure HDC1080
 */
shell_status_t cmd_hdc1080_config(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: hdc1080_config <option> [value]\r\n");
        Shell_Print("Options:\r\n");
        Shell_Print("  reset           - Reset sensor\r\n");
        Shell_Print("  heater <on/off> - Enable/disable heater\r\n");
        Shell_Print("  resolution <temp> <humi> - Set resolution (8,11,14)\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "reset") == 0) {
        HDC1080_Reset();
        HAL_Delay(20);
        Shell_PrintColored(COLOR_GREEN, "HDC1080 reset complete\r\n");
    }
    else if (strcmp(argv[1], "heater") == 0 && argc >= 3) {
        uint16_t config = HDC1080_GetConfig();
        if (strcmp(argv[2], "on") == 0) {
            config |= HDC1080_CONFIG_HEAT;
            Shell_PrintColored(COLOR_YELLOW, "HDC1080 heater enabled\r\n");
        } else if (strcmp(argv[2], "off") == 0) {
            config &= ~HDC1080_CONFIG_HEAT;
            Shell_PrintColored(COLOR_GREEN, "HDC1080 heater disabled\r\n");
        }
        HDC1080_SetConfig(config);
    }
    else if (strcmp(argv[1], "resolution") == 0 && argc >= 4) {
        uint16_t config = HDC1080_GetConfig();
        
        // Set temperature resolution
        if (strcmp(argv[2], "11") == 0) {
            config |= HDC1080_CONFIG_TRES;
        } else {
            config &= ~HDC1080_CONFIG_TRES;
        }
        
        // Set humidity resolution
        config &= ~HDC1080_CONFIG_HRES_MASK;
        if (strcmp(argv[3], "8") == 0) {
            config |= HDC1080_CONFIG_HRES_8;
        } else if (strcmp(argv[3], "11") == 0) {
            config |= HDC1080_CONFIG_HRES_11;
        } else {
            config |= HDC1080_CONFIG_HRES_14;
        }
        
        HDC1080_SetConfig(config);
        Shell_Printf("Resolution set to %s-bit temp, %s-bit humidity\r\n", argv[2], argv[3]);
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid configuration option\r\n");
    }
    
    return SHELL_OK;
}

/**
 * @brief Shell command to control HDC1080 monitoring
 */
shell_status_t cmd_hdc1080_monitor(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: hdc1080_monitor <on/off> [interval_ms]\r\n");
        Shell_Print("  on/off     - Enable/disable auto monitoring\r\n");
        Shell_Print("  interval_ms - Measurement interval in milliseconds\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "on") == 0) {
        if (argc >= 3) {
            uint16_t interval = (uint16_t)atoi(argv[2]);
            if (interval >= 100) {
                HDC1080_SetMeasurementInterval(interval);
            }
        }
        HDC1080_SetAutoMonitoring(1);
        Shell_PrintColored(COLOR_GREEN, "HDC1080 auto monitoring enabled\r\n");
        Shell_Printf("Measurement interval: %u ms\r\n", hdc1080_sensor.measurement_interval);
    }
    else if (strcmp(argv[1], "off") == 0) {
        HDC1080_SetAutoMonitoring(0);
        Shell_PrintColored(COLOR_YELLOW, "HDC1080 auto monitoring disabled\r\n");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid option. Use 'on' or 'off'\r\n");
    }
    
    return SHELL_OK;
}
