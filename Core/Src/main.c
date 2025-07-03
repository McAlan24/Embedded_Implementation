/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body with Advanced Terminal Shell
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpdma.h"
#include "i2c.h"
#include "memorymap.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Enhanced Shell Configuration */
#define SHELL_BUFFER_SIZE       256
#define SHELL_MAX_ARGS          10
#define SHELL_MAX_COMMANDS      30
#define SHELL_MAX_USERS         5
#define SHELL_USERNAME_LEN      16
#define SHELL_PASSWORD_LEN      16
#define SHELL_HISTORY_SIZE      20
#define SHELL_INPUT_BUFFER_SIZE 128

/* Ring Buffer Configuration */
#define UART_RX_BUFFER_SIZE     512
#define UART_TX_BUFFER_SIZE     1024

/* Timer LED configuration */
#define LED_TIMER_PATTERNS      8
#define LED_MAX_STEPS           16

/* Display Configuration */
#define DISPLAY_LINES           8
#define DISPLAY_CHARS_PER_LINE  21
#define DISPLAY_UPDATE_INTERVAL 1000  // ms

/* ANSI Escape Sequences */
#define ESC_CHAR                0x1B
#define ANSI_CURSOR_UP          "\033[A"
#define ANSI_CURSOR_DOWN        "\033[B"
#define ANSI_CURSOR_RIGHT       "\033[C"
#define ANSI_CURSOR_LEFT        "\033[D"
#define ANSI_CLEAR_LINE         "\033[2K"
#define ANSI_CURSOR_HOME        "\033[H"
#define ANSI_SAVE_CURSOR        "\033[s"
#define ANSI_RESTORE_CURSOR     "\033[u"

/* ANSI Colors */
#define COLOR_RESET       "\033[0m"
#define COLOR_RED         "\033[31m"
#define COLOR_GREEN       "\033[32m"
#define COLOR_YELLOW      "\033[33m"
#define COLOR_BLUE        "\033[34m"
#define COLOR_MAGENTA     "\033[35m"
#define COLOR_CYAN        "\033[36m"
#define COLOR_WHITE       "\033[37m"
#define COLOR_BOLD        "\033[1m"
#define COLOR_DIM         "\033[2m"
#define COLOR_BLINK       "\033[5m"

/* Enhanced Input States */
typedef enum {
    INPUT_STATE_NORMAL = 0,
    INPUT_STATE_ESCAPE,
    INPUT_STATE_BRACKET,
    INPUT_STATE_FUNCTION
} input_state_t;

/* LED Timer Patterns */
typedef enum {
    LED_PATTERN_OFF = 0,
    LED_PATTERN_STATIC,
    LED_PATTERN_BLINK,
    LED_PATTERN_SEQUENCE,
    LED_PATTERN_FADE,
    LED_PATTERN_CHASE,
    LED_PATTERN_RAINBOW,
    LED_PATTERN_BREATHE
} led_pattern_t;

/* Shell States */
typedef enum {
    SHELL_STATE_LOGIN,
    SHELL_STATE_PASSWORD,
    SHELL_STATE_AUTHENTICATED,
    SHELL_STATE_LOCKED
} shell_state_t;

/* User Privilege Levels */
typedef enum {
    USER_GUEST = 0,
    USER_USER = 1,
    USER_ADMIN = 2,
    USER_ROOT = 3
} user_privilege_t;

/* Shell Return Codes */
typedef enum {
    SHELL_OK = 0,
    SHELL_ERROR = -1,
    SHELL_INVALID_ARGS = -2,
    SHELL_COMMAND_NOT_FOUND = -3,
    SHELL_ACCESS_DENIED = -4,
    SHELL_EXIT = -5
} shell_status_t;

/* User Structure */
typedef struct {
    char username[SHELL_USERNAME_LEN];
    char password[SHELL_PASSWORD_LEN];
    user_privilege_t privilege;
    uint32_t login_count;
    uint32_t last_login;
    uint8_t active;
} shell_user_t;

/* Command Function Pointer */
typedef shell_status_t (*shell_command_func_t)(int argc, char **argv);

/* Command Structure */
typedef struct {
    const char *name;
    const char *description;
    shell_command_func_t function;
    const char *usage;
    user_privilege_t min_privilege;
} shell_command_t;

/* Session Structure */
typedef struct {
    shell_user_t *current_user;
    shell_state_t state;
    char current_input[SHELL_USERNAME_LEN];
    uint8_t input_index;
    uint32_t login_attempts;
    uint32_t session_start;
    uint32_t last_activity;
    uint8_t authenticated;
} shell_session_t;

/* LED Timer Structure */
typedef struct {
    led_pattern_t pattern;
    uint16_t speed;           // Timer period in ms
    uint8_t step;            // Current step in pattern
    uint8_t direction;       // Direction for fade/chase
    uint8_t intensity[3];    // LED intensities (if PWM available)
    uint8_t enabled;         // Timer enabled flag
    uint32_t last_update;    // Last update timestamp
} led_timer_ctx_t;

/* Ring Buffer Structure */
typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} ring_buffer_t;

/* Enhanced Line Editor */
typedef struct {
    char line[SHELL_INPUT_BUFFER_SIZE];
    uint16_t cursor_pos;     // Current cursor position
    uint16_t line_length;    // Current line length
    uint16_t display_start;  // Start of display window
    input_state_t input_state;
    uint8_t escape_buffer[8];
    uint8_t escape_index;
} line_editor_t;

/* Enhanced Shell Context */
typedef struct {
    ring_buffer_t rx_ring;
    char tx_buffer[UART_TX_BUFFER_SIZE];
    char rx_buffer[SHELL_BUFFER_SIZE];
    char history[SHELL_HISTORY_SIZE][SHELL_INPUT_BUFFER_SIZE];
    uint16_t rx_index;
    uint8_t rx_char;
    uint16_t history_index;
    uint16_t history_count;
    uint16_t history_current;
    line_editor_t editor;
    volatile uint8_t command_ready;
    shell_session_t session;
} enhanced_shell_ctx_t;

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
    DISPLAY_SCREEN_COUNT
} display_screen_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Global Variables */
static enhanced_shell_ctx_t shell_ctx;
static led_timer_ctx_t led_timer;
static display_context_t display_ctx;
static shell_command_t shell_commands[SHELL_MAX_COMMANDS];
static shell_user_t shell_users[SHELL_MAX_USERS];
static uint8_t command_count = 0;
static uint8_t user_count = 0;

/* System Variables */
volatile uint32_t button_press_count = 0;
volatile uint32_t last_interrupt_time = 0;
volatile uint8_t led_state = 0;
uint32_t debug_rx_count = 0;
uint32_t debug_cmd_count = 0;
uint32_t system_boot_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Terminal Function Prototypes */
void LED_Control(uint8_t state);
void Shell_Init(void);
void Shell_Process(void);
void Shell_Print(const char *str);
void Shell_PrintColored(const char *color, const char *str);
void Shell_Printf(const char *format, ...);
void Shell_RegisterCommand(const char *name, const char *description,
                          shell_command_func_t function, const char *usage,
                          user_privilege_t min_privilege);
void Shell_RegisterUser(const char *username, const char *password, user_privilege_t privilege);
void Shell_ShowPrompt(void);
void Shell_ShowLogin(void);
void Shell_ProcessLogin(void);
void Shell_ProcessPassword(void);
void Shell_ProcessCommand(void);
void Shell_AddToHistory(const char *command);
shell_user_t* Shell_FindUser(const char *username);
uint8_t Shell_VerifyPassword(shell_user_t *user, const char *password);
void Shell_Logout(void);
void Shell_ShowWelcome(void);
const char* Shell_GetPrivilegeString(user_privilege_t privilege);

void Shell_Enhanced_Init(void);
void Shell_Enhanced_Process(void);
void LineEditor_Init(void);
void LineEditor_ProcessChar(uint8_t ch);
void LineEditor_HandleEscape(uint8_t ch);
void LineEditor_MoveCursor(int16_t offset);
void LineEditor_InsertChar(uint8_t ch);
void LineEditor_DeleteChar(void);
void LineEditor_Redraw(void);
void LineEditor_HistoryUp(void);
void LineEditor_HistoryDown(void);
void LineEditor_Clear(void);
void LineEditor_HandleBackspace(void);
uint8_t RingBuffer_Put(ring_buffer_t *rb, uint8_t data);
uint8_t RingBuffer_Get(ring_buffer_t *rb, uint8_t *data);
uint16_t RingBuffer_Count(ring_buffer_t *rb);

/* Display Functions */
void Display_Init(void);
void Display_Update(void);
void Display_ShowWelcome(void);
void Display_ShowStatus(void);
void Display_ShowUserInfo(void);
void Display_ShowSystemInfo(void);
void Display_ShowActivity(void);
void Display_SetStatusLine(uint8_t line, const char* text);
void Display_ShowLoginAttempt(const char* username, uint8_t success);
void Display_ShowCommand(const char* command);
void Display_CycleScreen(void);

/* LED Timer Functions */
void LED_Timer_Init(void);
void LED_Timer_Start(led_pattern_t pattern, uint16_t speed);
void LED_Timer_Stop(void);
void LED_Timer_Update(void);
void LED_Timer_SetPattern(led_pattern_t pattern);
void LED_Pattern_Execute(void);

/* Enhanced Commands */
shell_status_t cmd_help(int argc, char **argv);
shell_status_t cmd_clear(int argc, char **argv);
shell_status_t cmd_whoami(int argc, char **argv);
shell_status_t cmd_uptime(int argc, char **argv);
shell_status_t cmd_history(int argc, char **argv);
shell_status_t cmd_logout(int argc, char **argv);
shell_status_t cmd_users(int argc, char **argv);
shell_status_t cmd_led(int argc, char **argv);
shell_status_t cmd_button(int argc, char **argv);
shell_status_t cmd_status(int argc, char **argv);
shell_status_t cmd_sysinfo(int argc, char **argv);
shell_status_t cmd_reset(int argc, char **argv);
shell_status_t cmd_debug(int argc, char **argv);
shell_status_t cmd_display(int argc, char **argv);
shell_status_t cmd_ledtimer(int argc, char **argv);
shell_status_t cmd_edit(int argc, char **argv);
shell_status_t cmd_screen(int argc, char **argv);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Ring Buffer Implementation */
uint8_t RingBuffer_Put(ring_buffer_t *rb, uint8_t data)
{
    if (rb->count >= UART_RX_BUFFER_SIZE) {
        return 0; // Buffer full
    }

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % UART_RX_BUFFER_SIZE;
    rb->count++;
    return 1;
}

uint8_t RingBuffer_Get(ring_buffer_t *rb, uint8_t *data)
{
    if (rb->count == 0) {
        return 0; // Buffer empty
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % UART_RX_BUFFER_SIZE;
    rb->count--;
    return 1;
}

uint16_t RingBuffer_Count(ring_buffer_t *rb)
{
    return rb->count;
}

/* Line Editor Implementation */
void LineEditor_Init(void)
{
    memset(&shell_ctx.editor, 0, sizeof(line_editor_t));
    shell_ctx.editor.input_state = INPUT_STATE_NORMAL;
}

void LineEditor_ProcessChar(uint8_t ch)
{
    switch (shell_ctx.editor.input_state) {
        case INPUT_STATE_NORMAL:
            if (ch == ESC_CHAR) {
                shell_ctx.editor.input_state = INPUT_STATE_ESCAPE;
                shell_ctx.editor.escape_index = 0;
            }
            else if (ch == '\r' || ch == '\n') {
                Shell_Print("\r\n");
                shell_ctx.editor.line[shell_ctx.editor.line_length] = '\0';
                shell_ctx.command_ready = 1;
            }
            else if (ch == '\b' || ch == 127) { // Backspace
                if (shell_ctx.editor.cursor_pos > 0) {
                    shell_ctx.editor.cursor_pos--;
                    // Delete character at cursor position
                    for (uint16_t i = shell_ctx.editor.cursor_pos; i < shell_ctx.editor.line_length - 1; i++) {
                        shell_ctx.editor.line[i] = shell_ctx.editor.line[i+1];
                    }
                    shell_ctx.editor.line_length--;

                    // Update display with improved backspace handling
                    LineEditor_HandleBackspace();
                }
            }
            else if (ch == '\t') { // Tab completion (future enhancement)
                // Tab completion logic here
            }
            else if (ch >= 32 && ch <= 126) { // Printable characters
                if (shell_ctx.session.state == SHELL_STATE_PASSWORD) {
                    Shell_Print("*");
                    LineEditor_InsertChar(ch);
                } else {
                    LineEditor_InsertChar(ch);
                    // For insert mode, we need to redraw to show the insertion
                    if (shell_ctx.editor.cursor_pos < shell_ctx.editor.line_length) {
                        LineEditor_Redraw();
                    } else {
                        Shell_Printf("%c", ch); // Just append at end
                    }
                }
            }
            break;

        case INPUT_STATE_ESCAPE:
            if (ch == '[') {
                shell_ctx.editor.input_state = INPUT_STATE_BRACKET;
            } else {
                shell_ctx.editor.input_state = INPUT_STATE_NORMAL;
            }
            break;

        case INPUT_STATE_BRACKET:
            LineEditor_HandleEscape(ch);
            shell_ctx.editor.input_state = INPUT_STATE_NORMAL;
            break;

        default:
            shell_ctx.editor.input_state = INPUT_STATE_NORMAL;
            break;
    }
}

void LineEditor_HandleBackspace(void)
{
    if (shell_ctx.session.state == SHELL_STATE_PASSWORD) {
        Shell_Print("\b \b"); // Simple backspace for password
        return;
    }

    // Move cursor back one position
    Shell_Print("\b");

    // Print remaining characters and a space to clear the last character
    for (uint16_t i = shell_ctx.editor.cursor_pos; i < shell_ctx.editor.line_length; i++) {
        Shell_Printf("%c", shell_ctx.editor.line[i]);
    }
    Shell_Print(" "); // Clear the last character

    // Move cursor back to correct position
    uint16_t moves_back = shell_ctx.editor.line_length - shell_ctx.editor.cursor_pos + 1;
    for (uint16_t i = 0; i < moves_back; i++) {
        Shell_Print("\b");
    }
}

void LineEditor_HandleEscape(uint8_t ch)
{
    switch (ch) {
        case 'A': // Up arrow
            LineEditor_HistoryUp();
            break;
        case 'B': // Down arrow
            LineEditor_HistoryDown();
            break;
        case 'C': // Right arrow
            if (shell_ctx.editor.cursor_pos < shell_ctx.editor.line_length) {
                shell_ctx.editor.cursor_pos++;
                Shell_Print(ANSI_CURSOR_RIGHT);
            }
            break;
        case 'D': // Left arrow
            if (shell_ctx.editor.cursor_pos > 0) {
                shell_ctx.editor.cursor_pos--;
                Shell_Print(ANSI_CURSOR_LEFT);
            }
            break;
        case 'H': // Home
            LineEditor_MoveCursor(-shell_ctx.editor.cursor_pos);
            break;
        case 'F': // End
            LineEditor_MoveCursor(shell_ctx.editor.line_length - shell_ctx.editor.cursor_pos);
            break;
    }
}

void LineEditor_MoveCursor(int16_t offset)
{
    int16_t new_pos = shell_ctx.editor.cursor_pos + offset;
    if (new_pos < 0) new_pos = 0;
    if (new_pos > shell_ctx.editor.line_length) new_pos = shell_ctx.editor.line_length;

    int16_t movement = new_pos - shell_ctx.editor.cursor_pos;
    shell_ctx.editor.cursor_pos = new_pos;

    if (movement > 0) {
        for (int i = 0; i < movement; i++) {
            Shell_Print(ANSI_CURSOR_RIGHT);
        }
    } else if (movement < 0) {
        for (int i = 0; i < -movement; i++) {
            Shell_Print(ANSI_CURSOR_LEFT);
        }
    }
}

void LineEditor_InsertChar(uint8_t ch)
{
    if (shell_ctx.editor.line_length >= SHELL_INPUT_BUFFER_SIZE - 1) {
        return; // Line too long
    }

    // Shift characters to the right
    for (uint16_t i = shell_ctx.editor.line_length; i > shell_ctx.editor.cursor_pos; i--) {
        shell_ctx.editor.line[i] = shell_ctx.editor.line[i-1];
    }

    shell_ctx.editor.line[shell_ctx.editor.cursor_pos] = ch;
    shell_ctx.editor.line_length++;
    shell_ctx.editor.cursor_pos++;
}

void LineEditor_DeleteChar(void)
{
    if (shell_ctx.editor.cursor_pos >= shell_ctx.editor.line_length) {
        return; // Nothing to delete
    }

    // Shift characters to the left
    for (uint16_t i = shell_ctx.editor.cursor_pos; i < shell_ctx.editor.line_length - 1; i++) {
        shell_ctx.editor.line[i] = shell_ctx.editor.line[i+1];
    }

    shell_ctx.editor.line_length--;

    // Update the terminal display
    LineEditor_Redraw();
}

void LineEditor_Redraw(void)
{
    if (shell_ctx.session.state == SHELL_STATE_PASSWORD) {
        return; // Don't redraw for password input
    }

    // Move cursor to beginning of line
    Shell_Print("\r");

    // Clear the entire line
    Shell_Print(ANSI_CLEAR_LINE);

    // Redraw prompt and current line content
    Shell_ShowPrompt();

    // Print the current line content
    for (uint16_t i = 0; i < shell_ctx.editor.line_length; i++) {
        Shell_Printf("%c", shell_ctx.editor.line[i]);
    }

    // Position cursor correctly by moving back to the cursor position
    uint16_t chars_after_cursor = shell_ctx.editor.line_length - shell_ctx.editor.cursor_pos;
    for (uint16_t i = 0; i < chars_after_cursor; i++) {
        Shell_Print(ANSI_CURSOR_LEFT);
    }
}
void LineEditor_HistoryUp(void)
{
    if (shell_ctx.history_count == 0) return;

    if (shell_ctx.history_current > 0) {
        shell_ctx.history_current--;
    }

    // Copy history command to current line
    strcpy(shell_ctx.editor.line, shell_ctx.history[shell_ctx.history_current]);
    shell_ctx.editor.line_length = strlen(shell_ctx.editor.line);
    shell_ctx.editor.cursor_pos = shell_ctx.editor.line_length;

    LineEditor_Redraw();
}

void LineEditor_HistoryDown(void)
{
    if (shell_ctx.history_current < shell_ctx.history_count - 1) {
        shell_ctx.history_current++;
        strcpy(shell_ctx.editor.line, shell_ctx.history[shell_ctx.history_current]);
        shell_ctx.editor.line_length = strlen(shell_ctx.editor.line);
        shell_ctx.editor.cursor_pos = shell_ctx.editor.line_length;
        LineEditor_Redraw();
    } else {
        // Clear current line for new input
        LineEditor_Clear();
    }
}

void LineEditor_Clear(void)
{
    memset(shell_ctx.editor.line, 0, sizeof(shell_ctx.editor.line));
    shell_ctx.editor.line_length = 0;
    shell_ctx.editor.cursor_pos = 0;
    shell_ctx.history_current = shell_ctx.history_count;
    LineEditor_Redraw();
}

/* LED Timer Implementation */
void LED_Timer_Init(void)
{
    memset(&led_timer, 0, sizeof(led_timer_ctx_t));
    led_timer.pattern = LED_PATTERN_OFF;
    led_timer.speed = 500; // 500ms default
}

void LED_Timer_Start(led_pattern_t pattern, uint16_t speed)
{
    led_timer.pattern = pattern;
    led_timer.speed = speed;
    led_timer.step = 0;
    led_timer.direction = 1;
    led_timer.enabled = 1;
    led_timer.last_update = HAL_GetTick();
}

void LED_Timer_Stop(void)
{
    led_timer.enabled = 0;
    led_timer.pattern = LED_PATTERN_OFF;
    LED_Control(0); // Turn off all LEDs
}

void LED_Timer_Update(void)
{
    if (!led_timer.enabled) return;

    uint32_t current_time = HAL_GetTick();
    if (current_time - led_timer.last_update >= led_timer.speed) {
        LED_Pattern_Execute();
        led_timer.last_update = current_time;
    }
}

void LED_Pattern_Execute(void)
{
    switch (led_timer.pattern) {
        case LED_PATTERN_OFF:
            LED_Control(0);
            break;

        case LED_PATTERN_STATIC:
            LED_Control(7); // All LEDs on
            break;

        case LED_PATTERN_BLINK:
            LED_Control(led_timer.step % 2 ? 7 : 0);
            led_timer.step++;
            break;

        case LED_PATTERN_SEQUENCE:
            LED_Control(1 << (led_timer.step % 3));
            led_timer.step++;
            break;

        case LED_PATTERN_CHASE:
            {
                uint8_t pattern = 1 << (led_timer.step % 3);
                LED_Control(pattern);
                led_timer.step++;
                if (led_timer.step >= 6) led_timer.step = 0;
            }
            break;

        case LED_PATTERN_BREATHE:
            {
                // Simulate breathing by varying speed
                if (led_timer.direction) {
                    led_timer.step++;
                    if (led_timer.step >= 10) led_timer.direction = 0;
                } else {
                    led_timer.step--;
                    if (led_timer.step == 0) led_timer.direction = 1;
                }
                LED_Control(led_timer.step > 5 ? 7 : 0);
            }
            break;

        case LED_PATTERN_RAINBOW:
            {
                uint8_t patterns[] = {1, 2, 4, 3, 5, 6, 7};
                LED_Control(patterns[led_timer.step % 7]);
                led_timer.step++;
            }
            break;

        default:
            LED_Control(0);
            break;
    }
}

/* Utility Functions */
void Shell_Print(const char *str)
{
    HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), 1000);
}

void Shell_PrintColored(const char *color, const char *str)
{
    HAL_UART_Transmit(&huart4, (uint8_t*)color, strlen(color), 1000);
    HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), 1000);
    HAL_UART_Transmit(&huart4, (uint8_t*)COLOR_RESET, strlen(COLOR_RESET), 1000);
}

void Shell_Printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(shell_ctx.tx_buffer, SHELL_BUFFER_SIZE, format, args);
    va_end(args);
    Shell_Print(shell_ctx.tx_buffer);
}

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
            "LED: %d", led_state);
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

void LED_Control(uint8_t state)
{
    switch(state) {
        case 0:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
            break;
        case 1:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
            break;
        case 7:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
            break;
    }
    led_state = state;
}

void Shell_RegisterUser(const char *username, const char *password, user_privilege_t privilege)
{
    if (user_count < SHELL_MAX_USERS) {
        strncpy(shell_users[user_count].username, username, SHELL_USERNAME_LEN - 1);
        strncpy(shell_users[user_count].password, password, SHELL_PASSWORD_LEN - 1);
        shell_users[user_count].privilege = privilege;
        shell_users[user_count].login_count = 0;
        shell_users[user_count].last_login = 0;
        shell_users[user_count].active = 1;
        user_count++;
    }
}

void Shell_RegisterCommand(const char *name, const char *description,
                          shell_command_func_t function, const char *usage,
                          user_privilege_t min_privilege)
{
    if (command_count < SHELL_MAX_COMMANDS) {
        shell_commands[command_count].name = name;
        shell_commands[command_count].description = description;
        shell_commands[command_count].function = function;
        shell_commands[command_count].usage = usage;
        shell_commands[command_count].min_privilege = min_privilege;
        command_count++;
    }
}

void Shell_ShowWelcome(void)
{
    Shell_Print("\r\n");
    Shell_PrintColored(COLOR_CYAN, "+================================================================+\r\n");
    Shell_PrintColored(COLOR_CYAN, "|              STM32H563 Advanced Shell Terminal                |\r\n");
    Shell_PrintColored(COLOR_CYAN, "+================================================================+\r\n");
    Shell_Printf("| System: STM32H563ZI @ %lu MHz                                 |\r\n", SystemCoreClock / 1000000);
    Shell_Printf("| Build:  %s %s                              |\r\n", __DATE__, __TIME__);
    Shell_PrintColored(COLOR_CYAN, "+================================================================+\r\n");
    Shell_PrintColored(COLOR_YELLOW, "              Welcome to the Secure Shell Terminal\r\n");
    Shell_PrintColored(COLOR_YELLOW, "                    Please authenticate\r\n\r\n");
    Shell_PrintColored(COLOR_WHITE, "Default users:\r\n");
    Shell_PrintColored(COLOR_WHITE, "  admin/admin123 (admin privileges)\r\n");
    Shell_PrintColored(COLOR_WHITE, "  user/user123   (user privileges)\r\n");
    Shell_PrintColored(COLOR_WHITE, "  guest/guest    (guest privileges)\r\n\r\n");
}

void Shell_ShowLogin(void)
{
    shell_ctx.session.state = SHELL_STATE_LOGIN;
    shell_ctx.session.input_index = 0;
    memset(shell_ctx.session.current_input, 0, sizeof(shell_ctx.session.current_input));
    Shell_PrintColored(COLOR_GREEN, "Login: ");
}

void Shell_ShowPrompt(void)
{
    if (shell_ctx.session.current_user) {
        if (shell_ctx.session.current_user->privilege >= USER_ADMIN) {
            Shell_PrintColored(COLOR_RED, "[");
            Shell_PrintColored(COLOR_YELLOW, shell_ctx.session.current_user->username);
            Shell_PrintColored(COLOR_RED, "@stm32h563");
            Shell_PrintColored(COLOR_RED, "]# ");
        } else {
            Shell_PrintColored(COLOR_GREEN, "[");
            Shell_PrintColored(COLOR_CYAN, shell_ctx.session.current_user->username);
            Shell_PrintColored(COLOR_GREEN, "@stm32h563");
            Shell_PrintColored(COLOR_GREEN, "]$ ");
        }
    }
}

shell_user_t* Shell_FindUser(const char *username)
{
    for (uint8_t i = 0; i < user_count; i++) {
        if (strcmp(shell_users[i].username, username) == 0 && shell_users[i].active) {
            return &shell_users[i];
        }
    }
    return NULL;
}

uint8_t Shell_VerifyPassword(shell_user_t *user, const char *password)
{
    return (strcmp(user->password, password) == 0);
}

const char* Shell_GetPrivilegeString(user_privilege_t privilege)
{
    switch (privilege) {
        case USER_GUEST: return "guest";
        case USER_USER: return "user";
        case USER_ADMIN: return "admin";
        case USER_ROOT: return "root";
        default: return "unknown";
    }
}

void Shell_ProcessLogin(void)
{
    if (strlen(shell_ctx.rx_buffer) > 0) {
        strncpy(shell_ctx.session.current_input, shell_ctx.rx_buffer, SHELL_USERNAME_LEN - 1);
        shell_ctx.session.state = SHELL_STATE_PASSWORD;
        Shell_PrintColored(COLOR_GREEN, "Password: ");
    } else {
        Shell_ShowLogin();
    }
}

void Shell_ProcessPassword(void)
{
    shell_user_t *user = Shell_FindUser(shell_ctx.session.current_input);

    if (user && Shell_VerifyPassword(user, shell_ctx.rx_buffer)) {
        shell_ctx.session.current_user = user;
        shell_ctx.session.authenticated = 1;
        shell_ctx.session.state = SHELL_STATE_AUTHENTICATED;
        shell_ctx.session.login_attempts = 0;
        shell_ctx.session.session_start = HAL_GetTick();

        user->login_count++;
        user->last_login = HAL_GetTick();

        Shell_Print("\r\n");
        Shell_PrintColored(COLOR_GREEN, "✓ Login successful!\r\n");
        Shell_Printf("Welcome %s! Privilege level: %s\r\n",
                    user->username, Shell_GetPrivilegeString(user->privilege));
        Shell_Printf("Session #%lu started.\r\n", user->login_count);
        Shell_Print("Type 'help' for available commands.\r\n\r\n");
        Shell_ShowPrompt();
        Display_ShowLoginAttempt(user->username, 1);
    } else {
        shell_ctx.session.login_attempts++;
        Shell_PrintColored(COLOR_RED, "\r\n✗ Authentication failed!\r\n");
        Display_ShowLoginAttempt(shell_ctx.session.current_input, 0);

        if (shell_ctx.session.login_attempts >= 3) {
            Shell_PrintColored(COLOR_RED, "Too many failed attempts. System locked.\r\n");
            shell_ctx.session.state = SHELL_STATE_LOCKED;
        } else {
            Shell_Printf("Attempt %lu/3. Try again.\r\n", shell_ctx.session.login_attempts);
            Shell_ShowLogin();
        }
    }
}

void Shell_Logout(void)
{
    if (shell_ctx.session.current_user) {
        uint32_t session_time = (HAL_GetTick() - shell_ctx.session.session_start) / 1000;
        Shell_Printf("Session ended. Duration: %lu seconds.\r\n", session_time);
        Shell_Printf("Goodbye %s!\r\n", shell_ctx.session.current_user->username);
    }

    shell_ctx.session.current_user = NULL;
    shell_ctx.session.authenticated = 0;
    shell_ctx.session.state = SHELL_STATE_LOGIN;
    shell_ctx.session.login_attempts = 0;

    Shell_Print("\r\n");
    Shell_ShowLogin();
}

void Shell_AddToHistory(const char *command)
{
    if (shell_ctx.history_count < SHELL_HISTORY_SIZE) {
        strcpy(shell_ctx.history[shell_ctx.history_count], command);
        shell_ctx.history_count++;
    } else {
        for (uint8_t i = 0; i < SHELL_HISTORY_SIZE - 1; i++) {
            strcpy(shell_ctx.history[i], shell_ctx.history[i + 1]);
        }
        strcpy(shell_ctx.history[SHELL_HISTORY_SIZE - 1], command);
    }
}

void Shell_ProcessCommand(void)
{
    if (strlen(shell_ctx.rx_buffer) > 0) {
        debug_cmd_count++;
        Shell_AddToHistory(shell_ctx.rx_buffer);

        char *argv[SHELL_MAX_ARGS];
        int argc = 0;
        char *token = strtok(shell_ctx.rx_buffer, " \t");

        while (token != NULL && argc < SHELL_MAX_ARGS) {
            argv[argc++] = token;
            token = strtok(NULL, " \t");
        }

        if (argc > 0) {
            uint8_t found = 0;
            Display_ShowCommand(argv[0]);
            for (uint8_t i = 0; i < command_count; i++) {
                if (strcmp(argv[0], shell_commands[i].name) == 0) {
                    if (shell_ctx.session.current_user->privilege >= shell_commands[i].min_privilege) {
                        shell_status_t status = shell_commands[i].function(argc, argv);
                        if (status == SHELL_EXIT) {
                            Shell_Logout();
                            return;
                        }
                    } else {
                        Shell_PrintColored(COLOR_RED, "✗ Access denied. Insufficient privileges.\r\n");
                        Shell_Printf("Required: %s, Current: %s\r\n",
                                    Shell_GetPrivilegeString(shell_commands[i].min_privilege),
                                    Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
                    }
                    found = 1;
                    break;
                }
            }

            if (!found) {
                Shell_PrintColored(COLOR_RED, "Command not found: ");
                Shell_Print(argv[0]);
                Shell_Print("\r\nType 'help' for available commands.\r\n");
            }
        }
    }

    Shell_ShowPrompt();
}

/* Enhanced Commands */
shell_status_t cmd_ledtimer(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_CYAN, "LED Timer Commands:\r\n");
        Shell_Print("  ledtimer start <pattern> [speed] - Start LED pattern\r\n");
        Shell_Print("  ledtimer stop                   - Stop LED timer\r\n");
        Shell_Print("  ledtimer status                 - Show timer status\r\n");
        Shell_Print("  ledtimer patterns               - List patterns\r\n");
        Shell_Print("\r\nPatterns: off, static, blink, sequence, chase, breathe, rainbow\r\n");
        return SHELL_OK;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (argc < 3) {
            Shell_PrintColored(COLOR_RED, "Usage: ledtimer start <pattern> [speed]\r\n");
            return SHELL_INVALID_ARGS;
        }

        led_pattern_t pattern = LED_PATTERN_OFF;
        uint16_t speed = 500; // Default 500ms

        if (strcmp(argv[2], "off") == 0) pattern = LED_PATTERN_OFF;
        else if (strcmp(argv[2], "static") == 0) pattern = LED_PATTERN_STATIC;
        else if (strcmp(argv[2], "blink") == 0) pattern = LED_PATTERN_BLINK;
        else if (strcmp(argv[2], "sequence") == 0) pattern = LED_PATTERN_SEQUENCE;
        else if (strcmp(argv[2], "chase") == 0) pattern = LED_PATTERN_CHASE;
        else if (strcmp(argv[2], "breathe") == 0) pattern = LED_PATTERN_BREATHE;
        else if (strcmp(argv[2], "rainbow") == 0) pattern = LED_PATTERN_RAINBOW;
        else {
            Shell_PrintColored(COLOR_RED, "Invalid pattern\r\n");
            return SHELL_INVALID_ARGS;
        }

        if (argc > 3) {
            speed = atoi(argv[3]);
            if (speed < 50) speed = 50; // Minimum 50ms
            if (speed > 5000) speed = 5000; // Maximum 5s
        }

        LED_Timer_Start(pattern, speed);
        Shell_PrintColored(COLOR_GREEN, "LED timer started\r\n");
        Shell_Printf("Pattern: %s, Speed: %d ms\r\n", argv[2], speed);
    }
    else if (strcmp(argv[1], "stop") == 0) {
        LED_Timer_Stop();
        Shell_PrintColored(COLOR_GREEN, "LED timer stopped\r\n");
    }
    else if (strcmp(argv[1], "status") == 0) {
        Shell_Printf("LED Timer Status:\r\n");
        Shell_Printf("  Enabled: %s\r\n", led_timer.enabled ? "Yes" : "No");
        Shell_Printf("  Pattern: %d\r\n", led_timer.pattern);
        Shell_Printf("  Speed: %d ms\r\n", led_timer.speed);
        Shell_Printf("  Step: %d\r\n", led_timer.step);
    }
    else if (strcmp(argv[1], "patterns") == 0) {
        Shell_PrintColored(COLOR_CYAN, "Available LED Patterns:\r\n");
        Shell_Print("  0: off      - All LEDs off\r\n");
        Shell_Print("  1: static   - All LEDs on\r\n");
        Shell_Print("  2: blink    - All LEDs blink together\r\n");
        Shell_Print("  3: sequence - LEDs light in sequence\r\n");
        Shell_Print("  4: chase    - Chasing light effect\r\n");
        Shell_Print("  5: breathe  - Breathing effect\r\n");
        Shell_Print("  6: rainbow  - Rainbow color sequence\r\n");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid ledtimer command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}

shell_status_t cmd_edit(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Enhanced Line Editor Features:\r\n");
    Shell_Print("  ↑/↓ arrows  - Navigate command history\r\n");
    Shell_Print("  ←/→ arrows  - Move cursor in line\r\n");
    Shell_Print("  Home/End    - Jump to line start/end\r\n");
    Shell_Print("  Backspace   - Delete character before cursor\r\n");
    Shell_Print("  Delete      - Delete character at cursor\r\n");
    Shell_Print("  Insert      - Insert characters at cursor\r\n");
    Shell_Printf("History: %d/%d commands stored\r\n",
                shell_ctx.history_count, SHELL_HISTORY_SIZE);
    return SHELL_OK;
}

/* Enhanced Shell Initialization */
void Shell_Enhanced_Init(void)
{
    // Initialize base shell
    Shell_Init();

    // Initialize enhanced components
    LineEditor_Init();
    LED_Timer_Init();

    // Initialize ring buffer
    memset(&shell_ctx.rx_ring, 0, sizeof(ring_buffer_t));

    // Register enhanced commands
    Shell_RegisterCommand("ledtimer", "Advanced LED timer control", cmd_ledtimer,
                         "ledtimer <start|stop|status|patterns> [args]", USER_USER);
    Shell_RegisterCommand("edit", "Line editor features", cmd_edit,
                         "edit", USER_GUEST);

}

/* Enhanced Main Process */
void Shell_Enhanced_Process(void)
{
    // Update LED timer patterns
    LED_Timer_Update();

    // Process commands if ready
    if (shell_ctx.command_ready) {
        shell_ctx.command_ready = 0;
        shell_ctx.session.last_activity = HAL_GetTick();

        // Copy editor line to command buffer for processing
        strcpy(shell_ctx.rx_buffer, shell_ctx.editor.line);

        switch (shell_ctx.session.state) {
            case SHELL_STATE_LOGIN:
                Shell_ProcessLogin();
                break;
            case SHELL_STATE_PASSWORD:
                Shell_ProcessPassword();
                break;
            case SHELL_STATE_AUTHENTICATED:
                Shell_ProcessCommand();
                break;
            case SHELL_STATE_LOCKED:
                Shell_PrintColored(COLOR_RED, "System locked. Restart required.\r\n");
                break;
        }

        // Reset editor for next command
        LineEditor_Clear();
    }
}

void Shell_Init(void)
{
    memset(&shell_ctx, 0, sizeof(shell_ctx));
    command_count = 0;
    user_count = 0;
    system_boot_time = HAL_GetTick();

    shell_ctx.session.state = SHELL_STATE_LOGIN;
    shell_ctx.session.current_user = NULL;
    shell_ctx.session.authenticated = 0;
    shell_ctx.session.login_attempts = 0;
    shell_ctx.session.session_start = HAL_GetTick();
    shell_ctx.session.last_activity = HAL_GetTick();

    Shell_RegisterUser("admin", "admin123", USER_ADMIN);
    Shell_RegisterUser("user", "user123", USER_USER);
    Shell_RegisterUser("guest", "guest", USER_GUEST);

    Shell_RegisterCommand("help", "Show available commands", cmd_help, "help [command]", USER_GUEST);
    Shell_RegisterCommand("clear", "Clear screen", cmd_clear, "clear", USER_GUEST);
    Shell_RegisterCommand("whoami", "Show current user", cmd_whoami, "whoami", USER_GUEST);
    Shell_RegisterCommand("uptime", "Show system uptime", cmd_uptime, "uptime", USER_GUEST);
    Shell_RegisterCommand("history", "Show command history", cmd_history, "history", USER_GUEST);
    Shell_RegisterCommand("logout", "Logout current user", cmd_logout, "logout", USER_GUEST);
    Shell_RegisterCommand("users", "List all users", cmd_users, "users", USER_USER);
    Shell_RegisterCommand("led", "Control LEDs", cmd_led, "led <0-7|on|off|blink> [led_num]", USER_USER);
    Shell_RegisterCommand("button", "Button information", cmd_button, "button", USER_GUEST);
    Shell_RegisterCommand("status", "System status", cmd_status, "status", USER_GUEST);
    Shell_RegisterCommand("sysinfo", "Detailed system info", cmd_sysinfo, "sysinfo", USER_USER);
    Shell_RegisterCommand("reset", "Reset system", cmd_reset, "reset [soft|hard]", USER_ADMIN);
    Shell_RegisterCommand("debug", "Debug information", cmd_debug, "debug", USER_ADMIN);
    Shell_RegisterCommand("display", "Control OLED display", cmd_display, "display <on|off|cycle|status>", USER_USER);
    Shell_RegisterCommand("screen", "Cycle display screens", cmd_screen, "screen", USER_GUEST);

    HAL_UART_Receive_IT(&huart4, &shell_ctx.rx_char, 1);

    HAL_Delay(100);
    Shell_ShowWelcome();
    Shell_ShowLogin();
}

void Shell_Process(void)
{
    if (shell_ctx.command_ready) {
        shell_ctx.command_ready = 0;
        shell_ctx.session.last_activity = HAL_GetTick();

        switch (shell_ctx.session.state) {
            case SHELL_STATE_LOGIN:
                Shell_ProcessLogin();
                break;
            case SHELL_STATE_PASSWORD:
                Shell_ProcessPassword();
                break;
            case SHELL_STATE_AUTHENTICATED:
                Shell_ProcessCommand();
                break;
            case SHELL_STATE_LOCKED:
                Shell_PrintColored(COLOR_RED, "System locked. Restart required.\r\n");
                break;
        }

        shell_ctx.rx_index = 0;
        memset(shell_ctx.rx_buffer, 0, sizeof(shell_ctx.rx_buffer));
    }
}

/* Command Implementations */
shell_status_t cmd_help(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Available Commands:\r\n");
    Shell_Print("===========================================\r\n");

    for (uint8_t i = 0; i < command_count; i++) {
        if (shell_ctx.session.current_user->privilege >= shell_commands[i].min_privilege) {
            Shell_Printf("%-12s - %s\r\n", shell_commands[i].name, shell_commands[i].description);
        }
    }

    Shell_Print("\r\nUse 'help <command>' for detailed usage.\r\n");
    Shell_Printf("Current privilege: %s\r\n",
                Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
    return SHELL_OK;
}

shell_status_t cmd_clear(int argc, char **argv)
{
    Shell_Print("\033[2J\033[H");
    return SHELL_OK;
}

shell_status_t cmd_whoami(int argc, char **argv)
{
    Shell_Printf("Username: %s\r\n", shell_ctx.session.current_user->username);
    Shell_Printf("Privilege: %s\r\n", Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
    Shell_Printf("Login count: %lu\r\n", shell_ctx.session.current_user->login_count);
    Shell_Printf("Session time: %lu seconds\r\n",
                (HAL_GetTick() - shell_ctx.session.session_start) / 1000);
    return SHELL_OK;
}

shell_status_t cmd_uptime(int argc, char **argv)
{
    uint32_t uptime_ms = HAL_GetTick();
    uint32_t uptime_sec = uptime_ms / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;

    Shell_PrintColored(COLOR_YELLOW, "System Uptime: ");
    Shell_Printf("%02lu:%02lu:%02lu\r\n", hours, minutes, seconds);
    return SHELL_OK;
}

shell_status_t cmd_screen(int argc, char **argv)
{
    Display_CycleScreen();
    Shell_Printf("Display screen cycled to: %d\r\n", display_ctx.current_screen);
    return SHELL_OK;
}

shell_status_t cmd_history(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Command History:\r\n");
    for (uint8_t i = 0; i < shell_ctx.history_count; i++) {
        Shell_Printf("%2d  %s\r\n", i + 1, shell_ctx.history[i]);
    }
    return SHELL_OK;
}

shell_status_t cmd_logout(int argc, char **argv)
{
    return SHELL_EXIT;
}

shell_status_t cmd_users(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Registered Users:\r\n");
    Shell_Print("Username     Privilege    Login Count  Status\r\n");
    Shell_Print("================================================\r\n");

    for (uint8_t i = 0; i < user_count; i++) {
        Shell_Printf("%-12s %-12s %-12lu %s\r\n",
                    shell_users[i].username,
                    Shell_GetPrivilegeString(shell_users[i].privilege),
                    shell_users[i].login_count,
                    shell_users[i].active ? "Active" : "Disabled");
    }
    return SHELL_OK;
}

shell_status_t cmd_led(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_RED, "Usage: led <0-7|on|off|blink> [led_num]\r\n");
        return SHELL_INVALID_ARGS;
    }

    if (strcmp(argv[1], "on") == 0) {
        int led_num = (argc > 2) ? atoi(argv[2]) : 1;
        if (led_num >= 1 && led_num <= 3) {
            GPIO_TypeDef* port = (led_num == 1) ? LED1_GPIO_Port :
                                (led_num == 2) ? LED2_GPIO_Port : LED3_GPIO_Port;
            uint16_t pin = (led_num == 1) ? LED1_Pin :
                          (led_num == 2) ? LED2_Pin : LED3_Pin;
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
            Shell_PrintColored(COLOR_GREEN, "LED turned ON\r\n");
            Shell_Printf("LED%d is now ON\r\n", led_num);
            display_ctx.last_update = 0;
        }
    }
    else if (strcmp(argv[1], "off") == 0) {
        int led_num = (argc > 2) ? atoi(argv[2]) : 1;
        if (led_num >= 1 && led_num <= 3) {
            GPIO_TypeDef* port = (led_num == 1) ? LED1_GPIO_Port :
                                (led_num == 2) ? LED2_GPIO_Port : LED3_GPIO_Port;
            uint16_t pin = (led_num == 1) ? LED1_Pin :
                          (led_num == 2) ? LED2_Pin : LED3_Pin;
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            Shell_PrintColored(COLOR_GREEN, "LED turned OFF\r\n");
            Shell_Printf("LED%d is now OFF\r\n", led_num);
        }
    }
    else if (strcmp(argv[1], "blink") == 0) {
        int led_num = (argc > 2) ? atoi(argv[2]) : 1;
        if (led_num >= 1 && led_num <= 3) {
            GPIO_TypeDef* port = (led_num == 1) ? LED1_GPIO_Port :
                                (led_num == 2) ? LED2_GPIO_Port : LED3_GPIO_Port;
            uint16_t pin = (led_num == 1) ? LED1_Pin :
                          (led_num == 2) ? LED2_Pin : LED3_Pin;

            Shell_Printf("Blinking LED%d...\r\n", led_num);
            for (int i = 0; i < 5; i++) {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
                HAL_Delay(200);
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
                HAL_Delay(200);
            }
            Shell_PrintColored(COLOR_GREEN, "Blink complete.\r\n");
        }
    }
    else {
        int state = atoi(argv[1]);
        if (state >= 0 && state <= 7) {
            LED_Control(state);
            Shell_PrintColored(COLOR_GREEN, "LED pattern set\r\n");
            Shell_Printf("LED pattern: %d\r\n", state);
        }
    }
    return SHELL_OK;
}

shell_status_t cmd_button(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Button Status:\r\n");
    Shell_Printf("Press count: %lu\r\n", button_press_count);
    Shell_Printf("Current state: %s\r\n",
                HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) ? "PRESSED" : "RELEASED");
    return SHELL_OK;
}

shell_status_t cmd_status(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "=== System Status ===\r\n");

    uint32_t uptime_sec = HAL_GetTick() / 1000;
    Shell_Printf("Uptime: %lu seconds\r\n", uptime_sec);
    Shell_Printf("Commands executed: %lu\r\n", debug_cmd_count);
    Shell_Printf("Button presses: %lu\r\n", button_press_count);
    Shell_Printf("Current LED state: %d\r\n", led_state);

    Shell_Printf("LEDs: LED1=%s, LED2=%s, LED3=%s\r\n",
        HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "ON" : "OFF",
        HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "ON" : "OFF",
        HAL_GPIO_ReadPin(LED3_GPIO_Port, LED3_Pin) ? "ON" : "OFF");

    return SHELL_OK;
}

shell_status_t cmd_sysinfo(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "=== System Information ===\r\n");
    Shell_Printf("MCU: STM32H563ZI\r\n");
    Shell_Printf("Core: ARM Cortex-M33\r\n");
    Shell_Printf("System Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    Shell_Printf("Flash: 2MB, SRAM: 640KB\r\n");
    Shell_Printf("Users: %d, Commands: %d\r\n", user_count, command_count);
    return SHELL_OK;
}

shell_status_t cmd_reset(int argc, char **argv)
{
    Shell_PrintColored(COLOR_YELLOW, "Resetting system in 3 seconds...\r\n");
    HAL_Delay(3000);
    NVIC_SystemReset();
    return SHELL_OK;
}

shell_status_t cmd_debug(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "=== Debug Information ===\r\n");
    Shell_Printf("RX Count: %lu\r\n", debug_rx_count);
    Shell_Printf("Command Count: %lu\r\n", debug_cmd_count);
    Shell_Printf("Session State: %d\r\n", shell_ctx.session.state);
    Shell_Printf("Login Attempts: %lu\r\n", shell_ctx.session.login_attempts);
    Shell_Printf("UART State: %lu\r\n", (unsigned long)HAL_UART_GetState(&huart4));
    return SHELL_OK;
}

/* Add Display Command to Shell */
shell_status_t cmd_display(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_CYAN, "Display Commands:\r\n");
        Shell_Print("  display on     - Enable display\r\n");
        Shell_Print("  display off    - Disable display\r\n");
        Shell_Print("  display cycle  - Cycle through screens\r\n");
        Shell_Print("  display status - Show current status\r\n");
        return SHELL_OK;
    }

    if (strcmp(argv[1], "on") == 0) {
        display_ctx.enabled = 1;
        Display_ShowStatus();
        Shell_PrintColored(COLOR_GREEN, "Display enabled\r\n");
    }
    else if (strcmp(argv[1], "off") == 0) {
        display_ctx.enabled = 0;
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();
        Shell_PrintColored(COLOR_GREEN, "Display disabled\r\n");
    }
    else if (strcmp(argv[1], "cycle") == 0) {
        Display_CycleScreen();
        Shell_PrintColored(COLOR_GREEN, "Display screen cycled\r\n");
    }
    else if (strcmp(argv[1], "status") == 0) {
        Shell_Printf("Display enabled: %s\r\n", display_ctx.enabled ? "Yes" : "No");
        Shell_Printf("Current screen: %d\r\n", display_ctx.current_screen);
        Shell_Printf("Last update: %lu ms ago\r\n",
                    HAL_GetTick() - display_ctx.last_update);
    }
    else {
        Shell_PrintColored(COLOR_RED, "Invalid display command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}

/* Enhanced UART Callback with Ring Buffer */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        debug_rx_count++;
        LineEditor_ProcessChar(shell_ctx.rx_char);
        HAL_UART_Receive_IT(&huart4, &shell_ctx.rx_char, 1);
    }
}

/* Timer Callback for LED patterns */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        LED_Timer_Update();
    }
}

/* Button Interrupt */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == BUTTON_Pin) {
        if ((current_time - last_interrupt_time) > 50) {
            button_press_count++;
            Display_CycleScreen();

            if (shell_ctx.session.authenticated) {
                Shell_Printf("\r\n[SYSTEM] Button #%lu pressed!\r\n", button_press_count);
                Shell_ShowPrompt();
            }

            last_interrupt_time = current_time;
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_USB_PCD_Init();
  MX_UART5_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  // Initialize LEDs to off state
  LED_Control(0);
  HAL_Delay(500);

  // Start timers
  HAL_TIM_Base_Start_IT(&htim6);

  // Initialize enhanced shell system
  Shell_Enhanced_Init();

  // Initialize OLED display
  Display_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // Process enhanced shell with line editing and LED timers
    Shell_Enhanced_Process();

    // Update OLED display
    Display_Update();

    // Small delay to prevent overwhelming the system
    HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 250;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
