/**
  ******************************************************************************
  * @file           : display_module.c
  * @brief          : OLED display implementation
  ******************************************************************************
  * @attention
  *
  * OLED Display Module for STM32H563ZI
  * Supports SSD1306 OLED with multiple screen displays
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "display_module.h"
#include "display_utils.h"
#include "shell_terminal.h"
#include "led_timer.h"
#include "system_config.h"
#include "hdc1080.h"
#include "adxl345.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
display_context_t display_ctx;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* Initialize Display */
void Display_Init(void)
{
    memset(&display_ctx, 0, sizeof(display_ctx));

    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    display_ctx.enabled = 1;
    display_ctx.current_screen = DISPLAY_SCREEN_STATUS;
    display_ctx.last_update = HAL_GetTick();

    Display_ShowWelcome();
    HAL_Delay(2000);  // Show welcome for 2 seconds
}

/* Show Welcome Screen */
void Display_ShowWelcome(void)
{
    if (!display_ctx.enabled) return;

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("HELLO Y'ALL", Font_7x10, White);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString("================", Font_6x8, White);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString("Advanced Shell", Font_7x10, White);
    ssd1306_SetCursor(0, 36);
    ssd1306_WriteString("System Ready", Font_7x10, White);
    ssd1306_SetCursor(0, 52);
    ssd1306_WriteString("Please Login...", Font_6x8, White);
    ssd1306_UpdateScreen();
}

/* Update Display - Call this in main loop */
void Display_Update(void)
{
    if (!display_ctx.enabled) return;

    uint32_t current_time = HAL_GetTick();
    if (current_time - display_ctx.last_update < DISPLAY_UPDATE_INTERVAL) {
        return;
    }

    display_ctx.last_update = current_time;

    switch (display_ctx.current_screen) {
        case DISPLAY_SCREEN_STATUS:
            Display_ShowStatus();
            break;
        case DISPLAY_SCREEN_USER_INFO:
            Display_ShowUserInfo();
            break;
        case DISPLAY_SCREEN_SYSTEM_INFO:
            Display_ShowSystemInfo();
            break;
        case DISPLAY_SCREEN_ACTIVITY:
            Display_ShowActivity();
            break;
        case DISPLAY_SCREEN_HDC1080_SENSOR:
            Display_ShowHDC1080Sensor();
            break;
        case DISPLAY_SCREEN_ADXL345_SENSOR:
            Display_ShowADXL345Sensor();
            break;
    }
}

/* Show Status Screen */
void Display_ShowStatus(void)
{
    ssd1306_Fill(Black);

    // Title
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== STATUS ===", Font_6x8, White);

    // Current User
    ssd1306_SetCursor(0, 12);
    if (shell_ctx.session.current_user) {
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
                "User: %s", shell_ctx.session.current_user->username);
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

        ssd1306_SetCursor(0, 20);
        snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2),
                "Priv: %s", Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
        ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);
    } else {
        ssd1306_WriteString("User: Not Logged In", Font_6x8, White);
    }

    // Uptime
    uint32_t uptime_sec = HAL_GetTick() / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    ssd1306_SetCursor(0, 28);
    snprintf(display_ctx.status_line3, sizeof(display_ctx.status_line3),
            "Up: %02luh %02lum", hours, minutes);
    ssd1306_WriteString(display_ctx.status_line3, Font_6x8, White);

    // Commands executed
    ssd1306_SetCursor(0, 36);
    snprintf(display_ctx.status_line4, sizeof(display_ctx.status_line4),
            "Cmds: %lu", debug_cmd_count);
    ssd1306_WriteString(display_ctx.status_line4, Font_6x8, White);

    // Button presses
    ssd1306_SetCursor(0, 44);
    snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
            "Button: %lu", button_press_count);
    ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

    // LED Status
    ssd1306_SetCursor(0, 52);
    snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2),
            "LED: %d", (int)led_state);
    ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);

    ssd1306_UpdateScreen();
}

/* Show User Info Screen */
void Display_ShowUserInfo(void)
{
    ssd1306_Fill(Black);

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("== USER INFO ==", Font_6x8, White);

    if (shell_ctx.session.current_user) {
        ssd1306_SetCursor(0, 12);
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
                "Name: %s", shell_ctx.session.current_user->username);
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

        ssd1306_SetCursor(0, 20);
        snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2),
                "Level: %s", Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
        ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);

        ssd1306_SetCursor(0, 28);
        snprintf(display_ctx.status_line3, sizeof(display_ctx.status_line3),
                "Logins: %lu", shell_ctx.session.current_user->login_count);
        ssd1306_WriteString(display_ctx.status_line3, Font_6x8, White);

        uint32_t session_time = (HAL_GetTick() - shell_ctx.session.session_start) / 1000;
        ssd1306_SetCursor(0, 36);
        snprintf(display_ctx.status_line4, sizeof(display_ctx.status_line4),
                "Session: %lus", session_time);
        ssd1306_WriteString(display_ctx.status_line4, Font_6x8, White);
    } else {
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString("No User Logged In", Font_6x8, White);
        ssd1306_SetCursor(0, 32);
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
                "Attempts: %lu/3", shell_ctx.session.login_attempts);
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);
    }

    ssd1306_UpdateScreen();
}

/* Show System Info Screen */
void Display_ShowSystemInfo(void)
{
    ssd1306_Fill(Black);

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("== SYSTEM ==", Font_6x8, White);

    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString("STM32H563ZI", Font_6x8, White);

    ssd1306_SetCursor(0, 20);
    snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
            "Clock: %luMHz", SystemCoreClock / 1000000);
    ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

    ssd1306_SetCursor(0, 28);
    snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2),
            "Users: %d", user_count);
    ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);

    ssd1306_SetCursor(0, 36);
    snprintf(display_ctx.status_line3, sizeof(display_ctx.status_line3),
            "Commands: %d", command_count);
    ssd1306_WriteString(display_ctx.status_line3, Font_6x8, White);

    ssd1306_SetCursor(0, 44);
    snprintf(display_ctx.status_line4, sizeof(display_ctx.status_line4),
            "RX Count: %lu", debug_rx_count);
    ssd1306_WriteString(display_ctx.status_line4, Font_6x8, White);

    ssd1306_UpdateScreen();
}

/* Show Activity Screen */
void Display_ShowActivity(void)
{
    ssd1306_Fill(Black);

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("== ACTIVITY ==", Font_6x8, White);

    // Show last few commands from history
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString("Recent Commands:", Font_6x8, White);

    uint8_t start_idx = (shell_ctx.history_count > 3) ? shell_ctx.history_count - 3 : 0;
    for (uint8_t i = 0; i < 3 && (start_idx + i) < shell_ctx.history_count; i++) {
        ssd1306_SetCursor(0, 20 + (i * 8));
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
                "> %s", shell_ctx.history[start_idx + i]);
        // Truncate if too long
        if (strlen(display_ctx.status_line1) > 20) {
            display_ctx.status_line1[20] = '\0';
        }
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);
    }

    ssd1306_UpdateScreen();
}

/* Show HDC1080 Temperature/Humidity Sensor Screen */
void Display_ShowHDC1080Sensor(void)
{
    ssd1306_Fill(Black);

    // Title
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("HDC1080 SENSOR", Font_6x8, White);
    ssd1306_SetCursor(0, 8);
    ssd1306_WriteString("==============", Font_6x8, White);

    hdc1080_sensor_t* sensor = HDC1080_GetSensorData();
    
    if (sensor && sensor->is_available) {
        // Update sensor reading
        HDC1080_Update();
        
        // Temperature
        ssd1306_SetCursor(0, 20);
        format_temperature(sensor->temperature, display_ctx.status_line1, sizeof(display_ctx.status_line1));
        ssd1306_WriteString(display_ctx.status_line1, Font_7x10, White);

        // Humidity
        ssd1306_SetCursor(0, 32);
        format_humidity(sensor->humidity, display_ctx.status_line2, sizeof(display_ctx.status_line2));
        ssd1306_WriteString(display_ctx.status_line2, Font_7x10, White);

        // Reading count
        ssd1306_SetCursor(0, 44);
        snprintf(display_ctx.status_line3, sizeof(display_ctx.status_line3),
                "Readings: %lu", sensor->readings_count);
        ssd1306_WriteString(display_ctx.status_line3, Font_6x8, White);

        // Auto monitoring status
        ssd1306_SetCursor(0, 56);
        snprintf(display_ctx.status_line4, sizeof(display_ctx.status_line4),
                "Auto: %s", sensor->auto_monitoring ? "ON" : "OFF");
        ssd1306_WriteString(display_ctx.status_line4, Font_6x8, White);
    } else {
        ssd1306_SetCursor(0, 24);
        ssd1306_WriteString("SENSOR ERROR", Font_7x10, White);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString("Not Available", Font_6x8, White);
    }

    ssd1306_UpdateScreen();
}

/* Show ADXL345 Accelerometer Screen */
void Display_ShowADXL345Sensor(void)
{
    ssd1306_Fill(Black);

    // Title
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("ADXL345 ACCEL", Font_6x8, White);
    ssd1306_SetCursor(0, 8);
    ssd1306_WriteString("==============", Font_6x8, White);

    adxl345_sensor_t* sensor = ADXL345_GetSensorData();
    
    if (sensor && sensor->is_available) {
        // Update sensor reading
        ADXL345_Update();
        
        // X-axis
        ssd1306_SetCursor(0, 20);
        format_acceleration(sensor->accel.x, display_ctx.status_line1, sizeof(display_ctx.status_line1));
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1), "X: %s", display_ctx.status_line1);
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

        // Y-axis
        ssd1306_SetCursor(0, 28);
        format_acceleration(sensor->accel.y, display_ctx.status_line2, sizeof(display_ctx.status_line2));
        snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2), "Y: %s", display_ctx.status_line2);
        ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);

        // Z-axis
        ssd1306_SetCursor(0, 36);
        format_acceleration(sensor->accel.z, display_ctx.status_line3, sizeof(display_ctx.status_line3));
        snprintf(display_ctx.status_line3, sizeof(display_ctx.status_line3), "Z: %s", display_ctx.status_line3);
        ssd1306_WriteString(display_ctx.status_line3, Font_6x8, White);

        // Magnitude
        ssd1306_SetCursor(0, 44);
        char mag_str[16];
        float_to_str(ADXL345_GetMagnitude(), mag_str, sizeof(mag_str), 2);
        snprintf(display_ctx.status_line4, sizeof(display_ctx.status_line4), "Mag: %sg", mag_str);
        ssd1306_WriteString(display_ctx.status_line4, Font_6x8, White);

        // Reading count
        ssd1306_SetCursor(0, 52);
        snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
                "Count: %lu", sensor->readings_count);
        ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);

        // Auto monitoring status
        ssd1306_SetCursor(0, 60);
        snprintf(display_ctx.status_line2, sizeof(display_ctx.status_line2),
                "Auto: %s", sensor->auto_monitoring ? "ON" : "OFF");
        ssd1306_WriteString(display_ctx.status_line2, Font_6x8, White);
    } else {
        ssd1306_SetCursor(0, 24);
        ssd1306_WriteString("SENSOR ERROR", Font_7x10, White);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString("Not Available", Font_6x8, White);
    }

    ssd1306_UpdateScreen();
}

/* Cycle through display screens */
void Display_CycleScreen(void)
{
    display_ctx.current_screen = (display_ctx.current_screen + 1) % DISPLAY_SCREEN_COUNT;
    display_ctx.last_update = 0; // Force immediate update
}

/* Show Login Attempt */
void Display_ShowLoginAttempt(const char* username, uint8_t success)
{
    if (!display_ctx.enabled) return;

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("LOGIN ATTEMPT", Font_7x10, White);
    ssd1306_SetCursor(0, 16);
    snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
            "User: %s", username);
    ssd1306_WriteString(display_ctx.status_line1, Font_6x8, White);
    ssd1306_SetCursor(0, 32);
    if (success) {
        ssd1306_WriteString("SUCCESS!", Font_7x10, White);
    } else {
        ssd1306_WriteString("FAILED!", Font_7x10, White);
    }
    ssd1306_UpdateScreen();
    HAL_Delay(1500); // Show for 1.5 seconds
}

/* Show Command Execution */
void Display_ShowCommand(const char* command)
{
    if (!display_ctx.enabled) return;

    // Temporarily show command execution
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("EXECUTING:", Font_6x8, White);
    ssd1306_SetCursor(0, 16);
    snprintf(display_ctx.status_line1, sizeof(display_ctx.status_line1),
            "> %s", command);
    if (strlen(display_ctx.status_line1) > 20) {
        display_ctx.status_line1[20] = '\0';
    }
    ssd1306_WriteString(display_ctx.status_line1, Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(500); // Show for 0.5 seconds
}

/* Set Status Line */
void Display_SetStatusLine(uint8_t line, const char* text)
{
    switch (line) {
        case 1:
            strncpy(display_ctx.status_line1, text, sizeof(display_ctx.status_line1) - 1);
            break;
        case 2:
            strncpy(display_ctx.status_line2, text, sizeof(display_ctx.status_line2) - 1);
            break;
        case 3:
            strncpy(display_ctx.status_line3, text, sizeof(display_ctx.status_line3) - 1);
            break;
        case 4:
            strncpy(display_ctx.status_line4, text, sizeof(display_ctx.status_line4) - 1);
            break;
    }
}
