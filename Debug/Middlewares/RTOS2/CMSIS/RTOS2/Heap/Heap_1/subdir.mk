################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/mcalan23/STM32Cube/Repository/Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c 

OBJS += \
./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.o 

C_DEPS += \
./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.o: /Users/mcalan23/STM32Cube/Repository/Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Device/ST/STM32H5xx/Include -I/Users/mcalan23/STM32Cube/Repository/STM32Cube_FW_H5_V1.5.0/Drivers/CMSIS/Include -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/include/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I/Users/mcalan23/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-FREERTOS/1.3.1/Drivers/CMSIS/RTOS2/Include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-RTOS2-2f-CMSIS-2f-RTOS2-2f-Heap-2f-Heap_1

clean-Middlewares-2f-RTOS2-2f-CMSIS-2f-RTOS2-2f-Heap-2f-Heap_1:
	-$(RM) ./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.cyclo ./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.d ./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.o ./Middlewares/RTOS2/CMSIS/RTOS2/Heap/Heap_1/heap_1.su

.PHONY: clean-Middlewares-2f-RTOS2-2f-CMSIS-2f-RTOS2-2f-Heap-2f-Heap_1

