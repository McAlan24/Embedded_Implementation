/**
  ******************************************************************************
  * @file           : shell_terminal.c
  * @brief          : Shell terminal implementation
  ******************************************************************************
  * @attention
  *
  * Advanced Shell Terminal System for STM32H563ZI
  * Supports user authentication, command processing, and line editing
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "shell_terminal.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
enhanced_shell_ctx_t shell_ctx;
shell_command_t shell_commands[SHELL_MAX_COMMANDS];
shell_user_t shell_users[SHELL_MAX_USERS];
uint8_t command_count = 0;
uint8_t user_count = 0;
uint32_t debug_rx_count = 0;
uint32_t debug_cmd_count = 0;

// UART receive buffer
static uint8_t uart_rx_byte;
static uint8_t uart_rx_enabled = 0;

// Temporary login storage
static char temp_username[SHELL_USERNAME_LEN];

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

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
        LineEditor_Redraw();
    }
}

void LineEditor_Clear(void)
{
    memset(shell_ctx.editor.line, 0, sizeof(shell_ctx.editor.line));
    shell_ctx.editor.line_length = 0;
    shell_ctx.editor.cursor_pos = 0;
    shell_ctx.history_current = shell_ctx.history_count;
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
    if (strlen(shell_ctx.session.current_input) > 0) {
        // Store username temporarily
        strncpy(temp_username, shell_ctx.session.current_input, SHELL_USERNAME_LEN - 1);
        temp_username[SHELL_USERNAME_LEN - 1] = '\0';
        
        // Switch to password mode
        shell_ctx.session.state = SHELL_STATE_PASSWORD;
        shell_ctx.session.input_index = 0;
        memset(shell_ctx.session.current_input, 0, sizeof(shell_ctx.session.current_input));
        Shell_Print("\r\n");
        Shell_PrintColored(COLOR_GREEN, "Password: ");
    } else {
        Shell_ShowLogin();
    }
}

void Shell_ProcessPassword(void)
{
    shell_user_t *user = Shell_FindUser(temp_username);

    if (user && Shell_VerifyPassword(user, shell_ctx.session.current_input)) {
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
    } else {
        shell_ctx.session.login_attempts++;
        Shell_PrintColored(COLOR_RED, "\r\n✗ Authentication failed!\r\n");

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
    if (strlen(shell_ctx.editor.line) > 0) {
        debug_cmd_count++;
        Shell_AddToHistory(shell_ctx.editor.line);

        char *argv[SHELL_MAX_ARGS];
        int argc = 0;
        char *token = strtok(shell_ctx.editor.line, " \t");

        while (token != NULL && argc < SHELL_MAX_ARGS) {
            argv[argc++] = token;
            token = strtok(NULL, " \t");
        }

        if (argc > 0) {
            uint8_t found = 0;
            for (uint8_t i = 0; i < command_count; i++) {
                if (strcmp(shell_commands[i].name, argv[0]) == 0) {
                    if (shell_ctx.session.current_user->privilege >= shell_commands[i].min_privilege) {
                        shell_commands[i].function(argc, argv);
                        found = 1;
                        break;
                    } else {
                        Shell_PrintColored(COLOR_RED, "Access denied: insufficient privileges\r\n");
                        found = 1;
                        break;
                    }
                }
            }

            if (!found) {
                Shell_PrintColored(COLOR_RED, "Command not found: ");
                Shell_Printf("%s\r\n", argv[0]);
                Shell_Print("Type 'help' for available commands.\r\n");
            }
        }
    }

    // Clear the line editor for next command
    LineEditor_Clear();
    Shell_ShowPrompt();
}

/* UART Interrupt Handling */
void Shell_UART_RxCallback(void)
{
    // Put received byte into ring buffer
    if (RingBuffer_Put(&shell_ctx.rx_ring, uart_rx_byte)) {
        debug_rx_count++;
    }
    
    // Continue receiving
    HAL_UART_Receive_IT(&huart4, &uart_rx_byte, 1);
}

void Shell_StartReceive(void)
{
    if (!uart_rx_enabled) {
        uart_rx_enabled = 1;
        HAL_UART_Receive_IT(&huart4, &uart_rx_byte, 1);
    }
}

void Shell_ProcessInput(void)
{
    uint8_t received_byte;
    
    // Process all available bytes in the ring buffer
    while (RingBuffer_Get(&shell_ctx.rx_ring, &received_byte)) {
        // Handle the received byte based on current shell state
        switch (shell_ctx.session.state) {
            case SHELL_STATE_LOGIN:
                if (received_byte == '\r' || received_byte == '\n') {
                    shell_ctx.session.current_input[shell_ctx.session.input_index] = '\0';
                    Shell_ProcessLogin();
                    shell_ctx.session.input_index = 0;
                    memset(shell_ctx.session.current_input, 0, sizeof(shell_ctx.session.current_input));
                } else if (received_byte == 0x7F || received_byte == 0x08) { // Backspace
                    if (shell_ctx.session.input_index > 0) {
                        shell_ctx.session.input_index--;
                        Shell_Print("\b \b");
                    }
                } else if (received_byte >= 32 && received_byte <= 126) { // Printable characters
                    if (shell_ctx.session.input_index < (SHELL_USERNAME_LEN - 1)) {
                        shell_ctx.session.current_input[shell_ctx.session.input_index++] = received_byte;
                        HAL_UART_Transmit(&huart4, &received_byte, 1, 100);
                    }
                }
                break;
                
            case SHELL_STATE_PASSWORD:
                if (received_byte == '\r' || received_byte == '\n') {
                    shell_ctx.session.current_input[shell_ctx.session.input_index] = '\0';
                    Shell_ProcessPassword();
                    shell_ctx.session.input_index = 0;
                    memset(shell_ctx.session.current_input, 0, sizeof(shell_ctx.session.current_input));
                } else if (received_byte == 0x7F || received_byte == 0x08) { // Backspace
                    if (shell_ctx.session.input_index > 0) {
                        shell_ctx.session.input_index--;
                        Shell_Print("\b \b");
                    }
                } else if (received_byte >= 32 && received_byte <= 126) { // Printable characters
                    if (shell_ctx.session.input_index < (SHELL_PASSWORD_LEN - 1)) {
                        shell_ctx.session.current_input[shell_ctx.session.input_index++] = received_byte;
                        Shell_Print("*"); // Echo asterisk for password
                    }
                }
                break;
                
            case SHELL_STATE_AUTHENTICATED:
                // Use line editor for command input
                LineEditor_ProcessChar(received_byte);
                
                // Check if command is ready
                if (shell_ctx.command_ready) {
                    shell_ctx.command_ready = 0;
                    Shell_ProcessCommand();
                }
                break;
                
            case SHELL_STATE_LOCKED:
                // Ignore input when locked
                break;
                
            default:
                break;
        }
    }
}

/* Enhanced Shell Initialization */
void Shell_Enhanced_Init(void)
{
    // Initialize base shell
    Shell_Init();

    // Initialize enhanced components
    LineEditor_Init();

    // Initialize ring buffer
    memset(&shell_ctx.rx_ring, 0, sizeof(ring_buffer_t));
}

/* Enhanced Main Process */
void Shell_Enhanced_Process(void)
{
    // Process commands if ready
    Shell_Process();
}

void Shell_Init(void)
{
    // Initialize shell context
    memset(&shell_ctx, 0, sizeof(enhanced_shell_ctx_t));
    shell_ctx.session.state = SHELL_STATE_LOGIN;
    shell_ctx.history_current = 0;

    // Register default users
    Shell_RegisterUser("admin", "admin123", USER_ADMIN);
    Shell_RegisterUser("user", "user123", USER_USER);
    Shell_RegisterUser("guest", "guest", USER_GUEST);

    // Register default commands
    Shell_RegisterCommand("help", "Show available commands", cmd_help, "help", USER_GUEST);
    Shell_RegisterCommand("clear", "Clear the screen", cmd_clear, "clear", USER_GUEST);
    Shell_RegisterCommand("whoami", "Show current user information", cmd_whoami, "whoami", USER_GUEST);
    Shell_RegisterCommand("uptime", "Show system uptime", cmd_uptime, "uptime", USER_GUEST);
    Shell_RegisterCommand("history", "Show command history", cmd_history, "history", USER_USER);
    Shell_RegisterCommand("logout", "Logout current user", cmd_logout, "logout", USER_GUEST);
    Shell_RegisterCommand("users", "List registered users", cmd_users, "users", USER_ADMIN);
    Shell_RegisterCommand("status", "Show system status", cmd_status, "status", USER_USER);
    Shell_RegisterCommand("sysinfo", "Show system information", cmd_sysinfo, "sysinfo", USER_GUEST);
    Shell_RegisterCommand("reset", "Reset the system", cmd_reset, "reset", USER_ADMIN);
    Shell_RegisterCommand("debug", "Show debug information", cmd_debug, "debug", USER_ADMIN);
    Shell_RegisterCommand("edit", "Show line editor features", cmd_edit, "edit", USER_GUEST);

    // Start UART receiving
    Shell_StartReceive();

    // Show welcome message
    Shell_ShowWelcome();
    Shell_ShowLogin();
}

void Shell_Process(void)
{
    // Process incoming UART data
    Shell_ProcessInput();
}

/* Basic Shell Commands */
shell_status_t cmd_help(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Available Commands:\r\n");
    Shell_Print("==================\r\n");
    
    for (uint16_t i = 0; i < command_count; i++) {
        if (shell_ctx.session.current_user->privilege >= shell_commands[i].min_privilege) {
            Shell_Printf("  %-12s - %s\r\n", shell_commands[i].name, shell_commands[i].description);
        }
    }
    
    Shell_Print("\r\nFor detailed usage: <command> --help\r\n");
    return SHELL_OK;
}

shell_status_t cmd_clear(int argc, char **argv)
{
    Shell_Print("\033[2J\033[H"); // Clear screen and move cursor to home
    return SHELL_OK;
}

shell_status_t cmd_whoami(int argc, char **argv)
{
    if (shell_ctx.session.current_user) {
        Shell_Printf("Current user: %s\r\n", shell_ctx.session.current_user->username);
        Shell_Printf("Privilege level: %s\r\n", Shell_GetPrivilegeString(shell_ctx.session.current_user->privilege));
        Shell_Printf("Login count: %lu\r\n", shell_ctx.session.current_user->login_count);
    } else {
        Shell_Print("Not logged in\r\n");
    }
    return SHELL_OK;
}

shell_status_t cmd_uptime(int argc, char **argv)
{
    uint32_t uptime_ms = HAL_GetTick();
    uint32_t uptime_sec = uptime_ms / 1000;
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    
    Shell_Printf("System uptime: %lu:%02lu:%02lu\r\n", hours, minutes, seconds);
    return SHELL_OK;
}

shell_status_t cmd_history(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Command History:\r\n");
    for (uint8_t i = 0; i < shell_ctx.history_count; i++) {
        Shell_Printf("  %d: %s\r\n", i + 1, shell_ctx.history[i]);
    }
    return SHELL_OK;
}

shell_status_t cmd_logout(int argc, char **argv)
{
    Shell_Logout();
    return SHELL_OK;
}

shell_status_t cmd_users(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Registered Users:\r\n");
    for (uint8_t i = 0; i < user_count; i++) {
        if (shell_users[i].active) {
            Shell_Printf("  %-12s - %s level, %lu logins\r\n", 
                        shell_users[i].username, 
                        Shell_GetPrivilegeString(shell_users[i].privilege),
                        shell_users[i].login_count);
        }
    }
    return SHELL_OK;
}

shell_status_t cmd_status(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "System Status:\r\n");
    Shell_Printf("  Uptime: %lu ms\r\n", HAL_GetTick());
    Shell_Printf("  Commands executed: %lu\r\n", debug_cmd_count);
    Shell_Printf("  RX characters: %lu\r\n", debug_rx_count);
    if (shell_ctx.session.current_user) {
        Shell_Printf("  Current user: %s\r\n", shell_ctx.session.current_user->username);
    }
    return SHELL_OK;
}

shell_status_t cmd_sysinfo(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "System Information:\r\n");
    Shell_Printf("  MCU: STM32H563ZI\r\n");
    Shell_Printf("  Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    Shell_Printf("  Build: %s %s\r\n", __DATE__, __TIME__);
    Shell_Printf("  Shell version: 2.0\r\n");
    return SHELL_OK;
}

shell_status_t cmd_reset(int argc, char **argv)
{
    Shell_PrintColored(COLOR_YELLOW, "System reset requested...\r\n");
    HAL_Delay(1000);
    NVIC_SystemReset();
    return SHELL_OK;
}

shell_status_t cmd_debug(int argc, char **argv)
{
    Shell_PrintColored(COLOR_CYAN, "Debug Information:\r\n");
    Shell_Printf("  Shell context size: %lu bytes\r\n", sizeof(enhanced_shell_ctx_t));
    Shell_Printf("  Command count: %u\r\n", command_count);
    Shell_Printf("  User count: %u\r\n", user_count);
    Shell_Printf("  History count: %u\r\n", shell_ctx.history_count);
    Shell_Printf("  RX ring buffer count: %u\r\n", RingBuffer_Count(&shell_ctx.rx_ring));
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
