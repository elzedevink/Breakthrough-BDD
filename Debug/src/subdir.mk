################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/sylvan_obj.cpp 

C_SRCS += \
../src/lace.c \
../src/sha2.c \
../src/sylvan_bdd.c \
../src/sylvan_cache.c \
../src/sylvan_common.c \
../src/sylvan_gmp.c \
../src/sylvan_ldd.c \
../src/sylvan_mt.c \
../src/sylvan_mtbdd.c \
../src/sylvan_refs.c \
../src/sylvan_sl.c \
../src/sylvan_stats.c \
../src/sylvan_table.c 

OBJS += \
./src/lace.o \
./src/sha2.o \
./src/sylvan_bdd.o \
./src/sylvan_cache.o \
./src/sylvan_common.o \
./src/sylvan_gmp.o \
./src/sylvan_ldd.o \
./src/sylvan_mt.o \
./src/sylvan_mtbdd.o \
./src/sylvan_obj.o \
./src/sylvan_refs.o \
./src/sylvan_sl.o \
./src/sylvan_stats.o \
./src/sylvan_table.o 

CPP_DEPS += \
./src/sylvan_obj.d 

C_DEPS += \
./src/lace.d \
./src/sha2.d \
./src/sylvan_bdd.d \
./src/sylvan_cache.d \
./src/sylvan_common.d \
./src/sylvan_gmp.d \
./src/sylvan_ldd.d \
./src/sylvan_mt.d \
./src/sylvan_mtbdd.d \
./src/sylvan_refs.d \
./src/sylvan_sl.d \
./src/sylvan_stats.d \
./src/sylvan_table.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../src -I../gmp -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../src -I../gmp -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


