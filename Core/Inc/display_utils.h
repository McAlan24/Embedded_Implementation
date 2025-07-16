/**
  ******************************************************************************
  * @file           : display_utils.h
  * @brief          : Display utilities for floating-point formatting
  ******************************************************************************
  */

#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert floating-point number to string without using floating-point printf
 * @param value: floating-point value to convert
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @param decimal_places: number of decimal places
 * @return pointer to buffer
 */
char* float_to_str(float value, char* buffer, size_t buffer_size, int decimal_places);

/**
 * @brief Format temperature for display
 * @param temperature: temperature value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_temperature(float temperature, char* buffer, size_t buffer_size);

/**
 * @brief Format humidity for display
 * @param humidity: humidity value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_humidity(float humidity, char* buffer, size_t buffer_size);

/**
 * @brief Format acceleration for display
 * @param accel: acceleration value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_acceleration(float accel, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_UTILS_H */
