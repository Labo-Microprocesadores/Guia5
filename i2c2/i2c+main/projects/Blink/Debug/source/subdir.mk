################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/AccelMagn_drv.c \
../source/App.c \
../source/Posicionamiento.c \
../source/SysTick.c \
../source/Timer.c \
../source/gpio.c \
../source/i2c.c 

OBJS += \
./source/AccelMagn_drv.o \
./source/App.o \
./source/Posicionamiento.o \
./source/SysTick.o \
./source/Timer.o \
./source/gpio.o \
./source/i2c.o 

C_DEPS += \
./source/AccelMagn_drv.d \
./source/App.d \
./source/Posicionamiento.d \
./source/SysTick.d \
./source/Timer.d \
./source/gpio.d \
./source/i2c.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


