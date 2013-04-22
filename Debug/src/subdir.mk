################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Endpoint.cpp \
../src/MT.cpp \
../src/Node.cpp \
../src/SSLSocketServer.cpp \
../src/SSLSocketSession.cpp \
../src/Server.cpp \
../src/SocketServer.cpp \
../src/SocketSession.cpp \
../src/ZNP.cpp \
../src/main.cpp 

OBJS += \
./src/Endpoint.o \
./src/MT.o \
./src/Node.o \
./src/SSLSocketServer.o \
./src/SSLSocketSession.o \
./src/Server.o \
./src/SocketServer.o \
./src/SocketSession.o \
./src/ZNP.o \
./src/main.o 

CPP_DEPS += \
./src/Endpoint.d \
./src/MT.d \
./src/Node.d \
./src/SSLSocketServer.d \
./src/SSLSocketSession.d \
./src/Server.d \
./src/SocketServer.d \
./src/SocketSession.d \
./src/ZNP.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/victor/android/cm/frameworks/native/include -I/home/victor/android/cm/external/openssl/include -I/home/victor/android/cm/system/core/include/cutils -I/home/victor/android/cm/external/stlport/stlport -I/usr/include/c++/4.6.3 -I/usr/include/c++/4.6 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


