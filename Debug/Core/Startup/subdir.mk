################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32h563zitx.s 

OBJS += \
./Core/Startup/startup_stm32h563zitx.o 

S_DEPS += \
./Core/Startup/startup_stm32h563zitx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -c -I../Core/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Device/ST/STM32H5xx/Include -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Include -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/include/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Drivers/CMSIS/RTOS2/Include/ -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h563zitx.d ./Core/Startup/startup_stm32h563zitx.o

.PHONY: clean-Core-2f-Startup

