/**
  ******************************************************************************
  * @file           : led_timer.c
  * @brief          : LED timer and pattern control implementation
  ******************************************************************************
  * @attention
  *
  * LED Timer Module for STM32H563ZI
  * Supports various LED patterns and timing control
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "led_timer.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
led_timer_ctx_t led_timer;
volatile uint8_t led_state = 0;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/**
 * @brief Control LED state
 */
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

void LED_Timer_SetPattern(led_pattern_t pattern)
{
    led_timer.pattern = pattern;
    led_timer.step = 0;
    led_timer.direction = 1;
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

/* LED Commands */

/**
 * @brief LED control command
 */
shell_status_t cmd_led(int argc, char **argv)
{
    if (argc < 2) {
        Shell_PrintColored(COLOR_CYAN, "LED Commands:\r\n");
        Shell_Print("  led on [1-3|all]  - Turn on LED(s)\r\n");
        Shell_Print("  led off [1-3|all] - Turn off LED(s)\r\n");
        Shell_Print("  led toggle [1-3]  - Toggle LED\r\n");
        Shell_Print("  led status        - Show LED status\r\n");
        return SHELL_OK;
    }

    if (strcmp(argv[1], "on") == 0) {
        if (argc > 2) {
            if (strcmp(argv[2], "all") == 0) {
                LED_Control(7);
                Shell_PrintColored(COLOR_GREEN, "All LEDs turned on\r\n");
            } else {
                int led_num = atoi(argv[2]);
                if (led_num >= 1 && led_num <= 3) {
                    LED_Control(led_num);
                    Shell_Printf("LED %d turned on\r\n", led_num);
                } else {
                    Shell_PrintColored(COLOR_RED, "Invalid LED number (1-3)\r\n");
                    return SHELL_INVALID_ARGS;
                }
            }
        } else {
            LED_Control(7);
            Shell_PrintColored(COLOR_GREEN, "All LEDs turned on\r\n");
        }
    }
    else if (strcmp(argv[1], "off") == 0) {
        LED_Control(0);
        Shell_PrintColored(COLOR_GREEN, "All LEDs turned off\r\n");
    }
    else if (strcmp(argv[1], "status") == 0) {
        Shell_PrintColored(COLOR_CYAN, "LED Status:\r\n");
        Shell_Printf("  Current state: %d\r\n", led_state);
        Shell_Printf("  LED1: %s\r\n", (led_state & 1) ? "ON" : "OFF");
        Shell_Printf("  LED2: %s\r\n", (led_state & 2) ? "ON" : "OFF");
        Shell_Printf("  LED3: %s\r\n", (led_state & 4) ? "ON" : "OFF");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Unknown LED command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}

/**
 * @brief LED Timer control command
 */
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
            if (speed < 50 || speed > 5000) {
                Shell_PrintColored(COLOR_RED, "Speed must be 50-5000 ms\r\n");
                return SHELL_INVALID_ARGS;
            }
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
        Shell_PrintColored(COLOR_CYAN, "LED Timer Status:\r\n");
        Shell_Printf("  Enabled: %s\r\n", led_timer.enabled ? "Yes" : "No");
        Shell_Printf("  Pattern: %d\r\n", led_timer.pattern);
        Shell_Printf("  Speed: %d ms\r\n", led_timer.speed);
        Shell_Printf("  Step: %d\r\n", led_timer.step);
        Shell_Printf("  Direction: %d\r\n", led_timer.direction);
    }
    else if (strcmp(argv[1], "patterns") == 0) {
        Shell_PrintColored(COLOR_CYAN, "Available LED Patterns:\r\n");
        Shell_Print("  0: off      - All LEDs off\r\n");
        Shell_Print("  1: static   - All LEDs on\r\n");
        Shell_Print("  2: blink    - All LEDs blink\r\n");
        Shell_Print("  3: sequence - LEDs light in sequence\r\n");
        Shell_Print("  4: chase    - Chasing pattern\r\n");
        Shell_Print("  5: breathe  - Breathing effect\r\n");
        Shell_Print("  6: rainbow  - Color cycling effect\r\n");
    }
    else {
        Shell_PrintColored(COLOR_RED, "Unknown ledtimer command\r\n");
        return SHELL_INVALID_ARGS;
    }

    return SHELL_OK;
}
