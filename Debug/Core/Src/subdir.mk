################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/adxl345.c \
../Core/Src/display_module.c \
../Core/Src/display_utils.c \
../Core/Src/gpdma.c \
../Core/Src/gpio.c \
../Core/Src/hdc1080.c \
../Core/Src/i2c.c \
../Core/Src/led_timer.c \
../Core/Src/main.c \
../Core/Src/memorymap.c \
../Core/Src/sensors_monitor.c \
../Core/Src/shell_terminal.c \
../Core/Src/ssd1306.c \
../Core/Src/ssd1306_fonts.c \
../Core/Src/stm32h5xx_hal_msp.c \
../Core/Src/stm32h5xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_config.c \
../Core/Src/system_stm32h5xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c \
../Core/Src/usb.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/adxl345.o \
./Core/Src/display_module.o \
./Core/Src/display_utils.o \
./Core/Src/gpdma.o \
./Core/Src/gpio.o \
./Core/Src/hdc1080.o \
./Core/Src/i2c.o \
./Core/Src/led_timer.o \
./Core/Src/main.o \
./Core/Src/memorymap.o \
./Core/Src/sensors_monitor.o \
./Core/Src/shell_terminal.o \
./Core/Src/ssd1306.o \
./Core/Src/ssd1306_fonts.o \
./Core/Src/stm32h5xx_hal_msp.o \
./Core/Src/stm32h5xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_config.o \
./Core/Src/system_stm32h5xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o \
./Core/Src/usb.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/adxl345.d \
./Core/Src/display_module.d \
./Core/Src/display_utils.d \
./Core/Src/gpdma.d \
./Core/Src/gpio.d \
./Core/Src/hdc1080.d \
./Core/Src/i2c.d \
./Core/Src/led_timer.d \
./Core/Src/main.d \
./Core/Src/memorymap.d \
./Core/Src/sensors_monitor.d \
./Core/Src/shell_terminal.d \
./Core/Src/ssd1306.d \
./Core/Src/ssd1306_fonts.d \
./Core/Src/stm32h5xx_hal_msp.d \
./Core/Src/stm32h5xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_config.d \
./Core/Src/system_stm32h5xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d \
./Core/Src/usb.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Device/ST/STM32H5xx/Include -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/adxl345.cyclo ./Core/Src/adxl345.d ./Core/Src/adxl345.o ./Core/Src/adxl345.su ./Core/Src/display_module.cyclo ./Core/Src/display_module.d ./Core/Src/display_module.o ./Core/Src/display_module.su ./Core/Src/display_utils.cyclo ./Core/Src/display_utils.d ./Core/Src/display_utils.o ./Core/Src/display_utils.su ./Core/Src/gpdma.cyclo ./Core/Src/gpdma.d ./Core/Src/gpdma.o ./Core/Src/gpdma.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/hdc1080.cyclo ./Core/Src/hdc1080.d ./Core/Src/hdc1080.o ./Core/Src/hdc1080.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/led_timer.cyclo ./Core/Src/led_timer.d ./Core/Src/led_timer.o ./Core/Src/led_timer.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/memorymap.cyclo ./Core/Src/memorymap.d ./Core/Src/memorymap.o ./Core/Src/memorymap.su ./Core/Src/sensors_monitor.cyclo ./Core/Src/sensors_monitor.d ./Core/Src/sensors_monitor.o ./Core/Src/sensors_monitor.su ./Core/Src/shell_terminal.cyclo ./Core/Src/shell_terminal.d ./Core/Src/shell_terminal.o ./Core/Src/shell_terminal.su ./Core/Src/ssd1306.cyclo ./Core/Src/ssd1306.d ./Core/Src/ssd1306.o ./Core/Src/ssd1306.su ./Core/Src/ssd1306_fonts.cyclo ./Core/Src/ssd1306_fonts.d ./Core/Src/ssd1306_fonts.o ./Core/Src/ssd1306_fonts.su ./Core/Src/stm32h5xx_hal_msp.cyclo ./Core/Src/stm32h5xx_hal_msp.d ./Core/Src/stm32h5xx_hal_msp.o ./Core/Src/stm32h5xx_hal_msp.su ./Core/Src/stm32h5xx_it.cyclo ./Core/Src/stm32h5xx_it.d ./Core/Src/stm32h5xx_it.o ./Core/Src/stm32h5xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_config.cyclo ./Core/Src/system_config.d ./Core/Src/system_config.o ./Core/Src/system_config.su ./Core/Src/system_stm32h5xx.cyclo ./Core/Src/system_stm32h5xx.d ./Core/Src/system_stm32h5xx.o ./Core/Src/system_stm32h5xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/usb.cyclo ./Core/Src/usb.d ./Core/Src/usb.o ./Core/Src/usb.su

.PHONY: clean-Core-2f-Src

