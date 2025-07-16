/**
  ******************************************************************************
  * @file           : led_timer.h
  * @brief          : Header for LED timer and pattern control
  ******************************************************************************
  * @attention
  *
  * LED Timer Module for STM32H563ZI
  * Supports various LED patterns and timing control
  *
  ******************************************************************************
  */

#ifndef __LED_TIMER_H
#define __LED_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shell_terminal.h"

/* Exported types ------------------------------------------------------------*/

/* Timer LED configuration */
#define LED_TIMER_PATTERNS      8
#define LED_MAX_STEPS           16

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

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* LED Control Functions */
void LED_Control(uint8_t state);

/* LED Timer Functions */
void LED_Timer_Init(void);
void LED_Timer_Start(led_pattern_t pattern, uint16_t speed);
void LED_Timer_Stop(void);
void LED_Timer_Update(void);
void LED_Timer_SetPattern(led_pattern_t pattern);
void LED_Pattern_Execute(void);

/* LED Timer Commands */
shell_status_t cmd_led(int argc, char **argv);
shell_status_t cmd_ledtimer(int argc, char **argv);

/* LED Timer Context Access */
extern led_timer_ctx_t led_timer;
extern volatile uint8_t led_state;

#ifdef __cplusplus
}
#endif

#endif /* __LED_TIMER_H */
