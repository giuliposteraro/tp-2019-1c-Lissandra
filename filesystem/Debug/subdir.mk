################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bibliotecaLFS.c \
../filesystem.c \
../funcionesLFS.c \
../servidor.c 

OBJS += \
./bibliotecaLFS.o \
./filesystem.o \
./funcionesLFS.o \
./servidor.o 

C_DEPS += \
./bibliotecaLFS.d \
./filesystem.d \
./funcionesLFS.d \
./servidor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-Soy-joven-y-quiero-vivir/biblioteca" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


