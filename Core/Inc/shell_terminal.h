/**
  ******************************************************************************
  * @file           : shell_terminal.h
  * @brief          : Header for shell terminal functionality
  ******************************************************************************
  * @attention
  *
  * Advanced Shell Terminal System for STM32H563ZI
  * Supports user authentication, command processing, and line editing
  *
  ******************************************************************************
  */

#ifndef __SHELL_TERMINAL_H
#define __SHELL_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* Exported types ------------------------------------------------------------*/

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

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Core Shell Functions */
void Shell_Init(void);
void Shell_Process(void);
void Shell_Enhanced_Init(void);
void Shell_Enhanced_Process(void);

/* UART Interrupt Functions */
void Shell_UART_RxCallback(void);
void Shell_StartReceive(void);
void Shell_ProcessInput(void);

/* Shell Output Functions */
void Shell_Print(const char *str);
void Shell_PrintColored(const char *color, const char *str);
void Shell_Printf(const char *format, ...);

/* Shell Management Functions */
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

/* Line Editor Functions */
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

/* Ring Buffer Functions */
uint8_t RingBuffer_Put(ring_buffer_t *rb, uint8_t data);
uint8_t RingBuffer_Get(ring_buffer_t *rb, uint8_t *data);
uint16_t RingBuffer_Count(ring_buffer_t *rb);

/* Command Functions */
shell_status_t cmd_help(int argc, char **argv);
shell_status_t cmd_clear(int argc, char **argv);
shell_status_t cmd_whoami(int argc, char **argv);
shell_status_t cmd_uptime(int argc, char **argv);
shell_status_t cmd_history(int argc, char **argv);
shell_status_t cmd_logout(int argc, char **argv);
shell_status_t cmd_users(int argc, char **argv);
shell_status_t cmd_status(int argc, char **argv);
shell_status_t cmd_sysinfo(int argc, char **argv);
shell_status_t cmd_reset(int argc, char **argv);
shell_status_t cmd_debug(int argc, char **argv);
shell_status_t cmd_edit(int argc, char **argv);

/* System Variables Access */
extern enhanced_shell_ctx_t shell_ctx;
extern shell_command_t shell_commands[];
extern shell_user_t shell_users[];
extern uint8_t command_count;
extern uint8_t user_count;
extern uint32_t debug_rx_count;
extern uint32_t debug_cmd_count;

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_TERMINAL_H */
