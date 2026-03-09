################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/Target/ethernetif.c 

OBJS += \
./LWIP/Target/ethernetif.o 

C_DEPS += \
./LWIP/Target/ethernetif.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/Target/%.o LWIP/Target/%.su LWIP/Target/%.cyclo: ../LWIP/Target/%.c LWIP/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../LWIP/App -I../LWIP/Target -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/system -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Drivers/STM32F7xx_HAL_Driver/Inc -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Drivers/BSP/Components/lan8742 -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/netif/ppp -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/apps/http -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Drivers/CMSIS/Device/ST/STM32F7xx/Include -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/lwip -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/lwip/apps -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/lwip/priv -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/lwip/prot -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/netif -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/compat/posix -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/compat/posix/net -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/src/include/compat/stdc -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Middlewares/Third_Party/LwIP/system/arch -IC:/Users/ellis/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.2/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-Target

clean-LWIP-2f-Target:
	-$(RM) ./LWIP/Target/ethernetif.cyclo ./LWIP/Target/ethernetif.d ./LWIP/Target/ethernetif.o ./LWIP/Target/ethernetif.su

.PHONY: clean-LWIP-2f-Target

