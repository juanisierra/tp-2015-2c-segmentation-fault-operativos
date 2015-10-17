################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librerias-sf/config.c \
../librerias-sf/listas.c \
../librerias-sf/sockets.c \
../librerias-sf/strings.c \
../librerias-sf/tiposDato.c 

OBJS += \
./librerias-sf/config.o \
./librerias-sf/listas.o \
./librerias-sf/sockets.o \
./librerias-sf/strings.o \
./librerias-sf/tiposDato.o 

C_DEPS += \
./librerias-sf/config.d \
./librerias-sf/listas.d \
./librerias-sf/sockets.d \
./librerias-sf/strings.d \
./librerias-sf/tiposDato.d 


# Each subdirectory must supply rules for building sources it contributes
librerias-sf/%.o: ../librerias-sf/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


