/**
  ******************************************************************************
  * @file           : display_utils.c
  * @brief          : Display utilities for floating-point formatting
  ******************************************************************************
  */

#include "display_utils.h"
#include <string.h>

/**
 * @brief Convert floating-point number to string without using floating-point printf
 * @param value: floating-point value to convert
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @param decimal_places: number of decimal places
 * @return pointer to buffer
 */
char* float_to_str(float value, char* buffer, size_t buffer_size, int decimal_places)
{
    if (buffer == NULL || buffer_size == 0) {
        return buffer;
    }
    
    // Handle negative values
    int negative = 0;
    if (value < 0) {
        negative = 1;
        value = -value;
    }
    
    // Extract integer part
    int integer_part = (int)value;
    
    // Extract fractional part
    float fractional_part = value - integer_part;
    
    // Calculate multiplier for decimal places
    int multiplier = 1;
    for (int i = 0; i < decimal_places; i++) {
        multiplier *= 10;
    }
    
    // Convert fractional part to integer
    int fractional_int = (int)(fractional_part * multiplier + 0.5f); // Round
    
    // Handle overflow in fractional part
    if (fractional_int >= multiplier) {
        integer_part++;
        fractional_int = 0;
    }
    
    // Format the string
    if (decimal_places > 0) {
        if (negative) {
            snprintf(buffer, buffer_size, "-%d.%0*d", integer_part, decimal_places, fractional_int);
        } else {
            snprintf(buffer, buffer_size, "%d.%0*d", integer_part, decimal_places, fractional_int);
        }
    } else {
        if (negative) {
            snprintf(buffer, buffer_size, "-%d", integer_part);
        } else {
            snprintf(buffer, buffer_size, "%d", integer_part);
        }
    }
    
    return buffer;
}

/**
 * @brief Format temperature for display
 * @param temperature: temperature value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_temperature(float temperature, char* buffer, size_t buffer_size)
{
    char temp_str[16];
    float_to_str(temperature, temp_str, sizeof(temp_str), 1);
    snprintf(buffer, buffer_size, "Temp: %s C", temp_str);
    return buffer;
}

/**
 * @brief Format humidity for display
 * @param humidity: humidity value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_humidity(float humidity, char* buffer, size_t buffer_size)
{
    char hum_str[16];
    float_to_str(humidity, hum_str, sizeof(hum_str), 1);
    snprintf(buffer, buffer_size, "Humidity: %s%%", hum_str);
    return buffer;
}

/**
 * @brief Format acceleration for display
 * @param accel: acceleration value
 * @param buffer: output buffer
 * @param buffer_size: size of output buffer
 * @return pointer to buffer
 */
char* format_acceleration(float accel, char* buffer, size_t buffer_size)
{
    char accel_str[16];
    float_to_str(accel, accel_str, sizeof(accel_str), 2);
    snprintf(buffer, buffer_size, "%s%sg", accel >= 0 ? "+" : "", accel_str);
    return buffer;
}
