/**
  ******************************************************************************
  * @file           : display_module.h
  * @brief          : Header for OLED display functionality
  ******************************************************************************
  * @attention
  *
  * OLED Display Module for STM32H563ZI
  * Supports SSD1306 OLED with multiple screen displays
  *
  ******************************************************************************
  */

#ifndef __DISPLAY_MODULE_H
#define __DISPLAY_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

/* Exported types ------------------------------------------------------------*/

/* Display Configuration */
#define DISPLAY_LINES           8
#define DISPLAY_CHARS_PER_LINE  21
#define DISPLAY_UPDATE_INTERVAL 1000  // ms

typedef struct {
    uint8_t enabled;
    uint32_t last_update;
    uint8_t current_screen;
    char status_line1[32];
    char status_line2[32];
    char status_line3[32];
    char status_line4[32];
} display_context_t;

typedef enum {
    DISPLAY_SCREEN_STATUS = 0,
    DISPLAY_SCREEN_USER_INFO,
    DISPLAY_SCREEN_SYSTEM_INFO,
    DISPLAY_SCREEN_ACTIVITY,
    DISPLAY_SCREEN_HDC1080_SENSOR,
    DISPLAY_SCREEN_ADXL345_SENSOR,
    DISPLAY_SCREEN_COUNT
} display_screen_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Display Functions */
void Display_Init(void);
void Display_Update(void);
void Display_ShowWelcome(void);
void Display_ShowStatus(void);
void Display_ShowUserInfo(void);
void Display_ShowSystemInfo(void);
void Display_ShowActivity(void);
void Display_ShowHDC1080Sensor(void);
void Display_ShowADXL345Sensor(void);
void Display_SetStatusLine(uint8_t line, const char* text);
void Display_ShowLoginAttempt(const char* username, uint8_t success);
void Display_ShowCommand(const char* command);
void Display_CycleScreen(void);

/* Display Context Access */
extern display_context_t display_ctx;

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_MODULE_H */
