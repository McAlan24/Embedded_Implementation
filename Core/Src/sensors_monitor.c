/**
  ******************************************************************************
  * @file           : sensors_monitor.c
  * @brief          : Sensors monitoring system implementation
  ******************************************************************************
  * @attention
  *
  * Sensors Monitoring System for STM32H563ZI
  * Handles auto-monitoring for all sensors
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sensors_monitor.h"
#include "shell_terminal.h"
#include "display_module.h"
#include "led_timer.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
sensors_monitor_t sensors_monitor;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/**
 * @brief Initialize sensors monitoring system
 */
void SensorsMonitor_Init(void)
{
    memset(&sensors_monitor, 0, sizeof(sensors_monitor_t));
    sensors_monitor.monitoring_active = 0;
}

/**
 * @brief Update all sensors based on their auto-monitoring settings
 */
void SensorsMonitor_Update(void)
{
    if (!sensors_monitor.monitoring_active) return;
    
    // Update HDC1080 sensor if auto-monitoring is enabled
    SensorsMonitor_UpdateHDC1080();
    
    // Update ADXL345 sensor if auto-monitoring is enabled
    SensorsMonitor_UpdateADXL345();
}

/**
 * @brief Update HDC1080 sensor
 */
void SensorsMonitor_UpdateHDC1080(void)
{
    hdc1080_sensor_t* hdc = HDC1080_GetSensorData();
    if (!hdc || !hdc->auto_monitoring) return;
    
    uint32_t current_time = HAL_GetTick();
    if (current_time - sensors_monitor.last_hdc1080_update >= hdc->measurement_interval) {
        HDC1080_Update();
        sensors_monitor.last_hdc1080_update = current_time;
        sensors_monitor.total_readings++;
    }
}

/**
 * @brief Update ADXL345 sensor
 */
void SensorsMonitor_UpdateADXL345(void)
{
    adxl345_sensor_t* adxl = ADXL345_GetSensorData();
    if (!adxl || !adxl->auto_monitoring) return;
    
    uint32_t current_time = HAL_GetTick();
    if (current_time - sensors_monitor.last_adxl345_update >= adxl->measurement_interval) {
        ADXL345_Update();
        sensors_monitor.last_adxl345_update = current_time;
        sensors_monitor.total_readings++;
    }
}

/**
 * @brief Set monitoring active state
 */
void SensorsMonitor_SetActive(uint8_t active)
{
    sensors_monitor.monitoring_active = active;
}

/**
 * @brief Check if monitoring is active
 */
uint8_t SensorsMonitor_IsActive(void)
{
    return sensors_monitor.monitoring_active;
}

/**
 * @brief Print monitoring status
 */
void SensorsMonitor_PrintStatus(void)
{
    Shell_PrintColored(COLOR_CYAN, "Sensors Monitor Status:\r\n");
    Shell_Printf("  Global Monitoring: %s\r\n", sensors_monitor.monitoring_active ? "ACTIVE" : "INACTIVE");
    Shell_Printf("  Total Readings: %lu\r\n", sensors_monitor.total_readings);
    
    Shell_PrintColored(COLOR_YELLOW, "\r\nIndividual Sensor Status:\r\n");
    
    // HDC1080 Status
    hdc1080_sensor_t* hdc = HDC1080_GetSensorData();
    if (hdc) {
        Shell_Printf("  HDC1080 Temp/Humidity: %s (Auto: %s)\r\n", 
                    hdc->is_available ? "Available" : "Not Available", 
                    hdc->auto_monitoring ? "ON" : "OFF");
        if (hdc->is_available && hdc->auto_monitoring) {
            Shell_Printf("    Interval: %u ms, Last Update: %lu ms ago\r\n", 
                        hdc->measurement_interval, 
                        HAL_GetTick() - sensors_monitor.last_hdc1080_update);
        }
    }
    
    // ADXL345 Status
    adxl345_sensor_t* adxl = ADXL345_GetSensorData();
    if (adxl) {
        Shell_Printf("  ADXL345 Accelerometer: %s (Auto: %s)\r\n", 
                    adxl->is_available ? "Available" : "Not Available", 
                    adxl->auto_monitoring ? "ON" : "OFF");
        if (adxl->is_available && adxl->auto_monitoring) {
            Shell_Printf("    Interval: %u ms, Last Update: %lu ms ago\r\n", 
                        adxl->measurement_interval, 
                        HAL_GetTick() - sensors_monitor.last_adxl345_update);
        }
    }
}

/**
 * @brief Get sensors monitor context
 */
sensors_monitor_t* SensorsMonitor_GetContext(void)
{
    return &sensors_monitor;
}

/* Shell Commands */

/**
 * @brief Shell command to control sensors monitoring
 */
shell_status_t cmd_sensors_monitor(int argc, char **argv)
{
    if (argc < 2) {
        Shell_Print("Usage: sensors_monitor <on|off|status>\r\n");
        Shell_Print("  on     - Enable global sensors monitoring\r\n");
        Shell_Print("  off    - Disable global sensors monitoring\r\n");
        Shell_Print("  status - Show monitoring status\r\n");
        return SHELL_OK;
    }
    
    if (strcmp(argv[1], "on") == 0) {
        SensorsMonitor_SetActive(1);
        Shell_PrintColored(COLOR_GREEN, "Sensors monitoring enabled\r\n");
    }
    else if (strcmp(argv[1], "off") == 0) {
        SensorsMonitor_SetActive(0);
        Shell_PrintColored(COLOR_YELLOW, "Sensors monitoring disabled\r\n");
    }
    else if (strcmp(argv[1], "status") == 0) {
        SensorsMonitor_PrintStatus();
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid option. Use 'on', 'off', or 'status'\r\n");
    }
    
    return SHELL_OK;
}

/**
 * @brief Shell command to show all sensors status
 */
shell_status_t cmd_sensors_status(int argc, char **argv)
{
    SensorsMonitor_PrintStatus();
    return SHELL_OK;
}

/**
 * @brief Shell command to read all sensors at once
 */
shell_status_t cmd_sensors_all(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "All Sensors Reading:\r\n");
    Shell_Print("========================\r\n");
    
    // HDC1080 Temperature/Humidity Sensor
    Shell_PrintColored(COLOR_YELLOW, "HDC1080 Temp/Humidity:\r\n");
    hdc1080_sensor_t* hdc = HDC1080_GetSensorData();
    if (hdc && hdc->is_available) {
        HDC1080_Update();
        Shell_Printf("  Temperature: %.2f°C\r\n", hdc->temperature);
        Shell_Printf("  Humidity: %.2f%%\r\n", hdc->humidity);
    } else {
        Shell_Print("  Not available\r\n");
    }
    
    // ADXL345 Accelerometer
    Shell_PrintColored(COLOR_YELLOW, "\r\nADXL345 Accelerometer:\r\n");
    adxl345_sensor_t* adxl = ADXL345_GetSensorData();
    if (adxl && adxl->is_available) {
        ADXL345_Update();
        Shell_Printf("  X-axis: %+.3f g\r\n", adxl->accel.x);
        Shell_Printf("  Y-axis: %+.3f g\r\n", adxl->accel.y);
        Shell_Printf("  Z-axis: %+.3f g\r\n", adxl->accel.z);
        Shell_Printf("  Magnitude: %.3f g\r\n", ADXL345_GetMagnitude());
    } else {
        Shell_Print("  Not available\r\n");
    }
    
    return SHELL_OK;
}
