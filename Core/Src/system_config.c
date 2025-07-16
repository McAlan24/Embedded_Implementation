/**
  ******************************************************************************
  * @file           : system_config.c
  * @brief          : System configuration and common definitions
  ********        Shell_Printf("  Available screens: %d\r\n", DISPLAY_SCREEN_COUNT);
        Shell_Print("  0: Status\r\n");
        Shell_Print("  1: User Info\r\n");
        Shell_Print("  2: System Info\r\n");
        Shell_Print("  3: Activity\r\n");
        Shell_Print("  4: HDC1080 Sensor\r\n");
        Shell_Print("  5: ADXL345 Sensor\r\n");****************************************************************
  * @attention
  *
  * System Configuration Module for STM32H563ZI
  * Contains common definitions and system variables
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "system_config.h"
#include "shell_terminal.h"
#include "display_module.h"
#include "led_timer.h"
#include "hdc1080.h"
#include "adxl345.h"
#include "sensors_monitor.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile uint32_t button_press_count = 0;
volatile uint32_t last_interrupt_time = 0;
uint32_t system_boot_time = 0;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/**
 * @brief Initialize system modules
 */
void System_Init(void)
{
    system_boot_time = HAL_GetTick();
    
    // Initialize LED Timer
    LED_Timer_Init();
    
    // Initialize HDC1080 Temperature/Humidity Sensor
    HDC1080_Init();
    
    // Initialize ADXL345 Accelerometer
    ADXL345_Init();
    
    // Initialize Sensors Monitor
    SensorsMonitor_Init();
    
    // Initialize Display
    Display_Init();
    
    // Initialize Shell Terminal
    Shell_Enhanced_Init();
    
    // Register all commands
    Shell_RegisterCommand("help", "Show available commands", cmd_help, "help", USER_GUEST);
//    Shell_RegisterCommand("clear", "Clear screen", cmd_clear, "clear", USER_GUEST);
//    Shell_RegisterCommand("whoami", "Show current user", cmd_whoami, "whoami", USER_GUEST);
//    Shell_RegisterCommand("uptime", "Show system uptime", cmd_uptime, "uptime", USER_GUEST);
//    Shell_RegisterCommand("history", "Show command history", cmd_history, "history", USER_USER);
//    Shell_RegisterCommand("logout", "Logout current user", cmd_logout, "logout", USER_GUEST);
//    Shell_RegisterCommand("users", "Show registered users", cmd_users, "users", USER_ADMIN);
//    Shell_RegisterCommand("status", "Show system status", cmd_status, "status", USER_USER);
//    Shell_RegisterCommand("sysinfo", "Show system information", cmd_sysinfo, "sysinfo", USER_USER);
//    Shell_RegisterCommand("reset", "Reset system", cmd_reset, "reset", USER_ADMIN);
//    Shell_RegisterCommand("debug", "Show debug information", cmd_debug, "debug", USER_ADMIN);
//    Shell_RegisterCommand("edit", "Line editor info", cmd_edit, "edit", USER_USER);
//
    // LED Commands
    Shell_RegisterCommand("led", "LED control", cmd_led, "led <on|off|status> [1-3|all]", USER_USER);
    Shell_RegisterCommand("ledtimer", "LED timer control", cmd_ledtimer, "ledtimer <start|stop|status|patterns> [args]", USER_USER);
    
    // HDC1080 Temperature/Humidity Sensor Commands
    Shell_RegisterCommand("temp", "Temperature/humidity sensor", cmd_hdc1080_read, "temp", USER_USER);
    Shell_RegisterCommand("hdc1080", "HDC1080 sensor control", cmd_hdc1080_read, "hdc1080", USER_USER);
    Shell_RegisterCommand("hdc1080_status", "HDC1080 sensor status", cmd_hdc1080_status, "hdc1080_status", USER_USER);
    Shell_RegisterCommand("hdc1080_config", "HDC1080 configuration", cmd_hdc1080_config, "hdc1080_config <option> [value]", USER_ADMIN);
    Shell_RegisterCommand("hdc1080_monitor", "HDC1080 monitoring", cmd_hdc1080_monitor, "hdc1080_monitor <on|off> [interval]", USER_USER);
    
    // ADXL345 Accelerometer Commands
    Shell_RegisterCommand("accel", "Accelerometer sensor", cmd_adxl345_read, "accel", USER_USER);
    Shell_RegisterCommand("adxl345", "ADXL345 sensor control", cmd_adxl345_read, "adxl345", USER_USER);
    Shell_RegisterCommand("adxl345_status", "ADXL345 sensor status", cmd_adxl345_status, "adxl345_status", USER_USER);
    Shell_RegisterCommand("adxl345_config", "ADXL345 configuration", cmd_adxl345_config, "adxl345_config <option> [value]", USER_ADMIN);
    Shell_RegisterCommand("adxl345_monitor", "ADXL345 monitoring", cmd_adxl345_monitor, "adxl345_monitor <on|off> [interval]", USER_USER);
    Shell_RegisterCommand("adxl345_motion", "ADXL345 motion detection", cmd_adxl345_motion, "adxl345_motion <option> [parameters]", USER_USER);
    Shell_RegisterCommand("adxl345_calibrate", "ADXL345 calibration", cmd_adxl345_calibrate, "adxl345_calibrate", USER_ADMIN);
    
    // Sensors Monitor Commands
    Shell_RegisterCommand("sensors", "All sensors reading", cmd_sensors_all, "sensors", USER_USER);
    Shell_RegisterCommand("sensors_monitor", "Sensors monitoring control", cmd_sensors_monitor, "sensors_monitor <on|off|status>", USER_USER);
    Shell_RegisterCommand("sensors_status", "Sensors status", cmd_sensors_status, "sensors_status", USER_USER);
    
    // System Commands
    Shell_RegisterCommand("button", "Button status", cmd_button, "button", USER_USER);
    Shell_RegisterCommand("display", "Display control", cmd_display, "display <on|off|info>", USER_USER);
    Shell_RegisterCommand("screen", "Screen control", cmd_screen, "screen <next|info>", USER_USER);
}

/**
 * @brief Main system processing loop
 */
void System_Process(void)
{
    // Update LED timer patterns
    LED_Timer_Update();
    
    // Process shell commands
    Shell_Enhanced_Process();
    
    // Update display
    Display_Update();
    
    // Update sensors monitor (handles auto-monitoring for all sensors)
    SensorsMonitor_Update();
}

/* System Commands */

/**
 * @brief Button status command
 */
shell_status_t cmd_button(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Button Status:\r\n");
    Shell_Printf("  Press count: %lu\r\n", button_press_count);
    Shell_Printf("  Last interrupt: %lu ms ago\r\n", 
                HAL_GetTick() - last_interrupt_time);
    return SHELL_OK;
}

/**
 * @brief Display control command
 */
shell_status_t cmd_display(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_CYAN, "Display Commands:\r\n");
        Shell_Print("  display on    - Enable display\r\n");
        Shell_Print("  display off   - Disable display\r\n");
        Shell_Print("  display info  - Show display info\r\n");
        return SHELL_OK;
    }

    if (strcmp(argv[1], "on") == 0) {
        display_ctx.enabled = 1;
        Shell_PrintColored(COLOR_GREEN, "Display enabled\r\n");
    }
    else if (strcmp(argv[1], "off") == 0) {
        display_ctx.enabled = 0;
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();
        Shell_PrintColored(COLOR_YELLOW, "Display disabled\r\n");
    }
    else if (strcmp(argv[1], "info") == 0) {
        Shell_PrintColored(COLOR_CYAN, "Display Information:\r\n");
        Shell_Printf("  Status: %s\r\n", display_ctx.enabled ? "Enabled" : "Disabled");
        Shell_Printf("  Current screen: %d\r\n", display_ctx.current_screen);
        Shell_Printf("  Last update: %lu ms ago\r\n", 
                    HAL_GetTick() - display_ctx.last_update);
        Shell_Printf("  Update interval: %d ms\r\n", DISPLAY_UPDATE_INTERVAL);
    }
    else {
        Shell_PrintColored(COLOR_RED, "Unknown display command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}

/**
 * @brief Screen control command
 */
shell_status_t cmd_screen(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_CYAN, "Screen Commands:\r\n");
        Shell_Print("  screen next   - Next screen\r\n");
        Shell_Print("  screen info   - Show screen info\r\n");
        return SHELL_OK;
    }

    if (strcmp(argv[1], "next") == 0) {
        Display_CycleScreen();
        Shell_Printf("Switched to screen %d\r\n", display_ctx.current_screen);
    }
    else if (strcmp(argv[1], "info") == 0) {
        Shell_PrintColored(COLOR_CYAN, "Screen Information:\r\n");
        Shell_Printf("  Current screen: %d\r\n", display_ctx.current_screen);
        Shell_Printf("  Available screens: %d\r\n", DISPLAY_SCREEN_COUNT);
        Shell_Print("  0: Status\r\n");
        Shell_Print("  1: User Info\r\n");
        Shell_Print("  2: System Info\r\n");
        Shell_Print("  3: Activity\r\n");
        Shell_Print("  4: HDC1080 Sensor\r\n");
        Shell_Print("  5: ADXL345 Sensor\r\n");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Unknown screen command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}
