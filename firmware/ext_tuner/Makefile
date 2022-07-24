######################################
# target
######################################
TARGET = tuner


######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
OPT = -O2


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
syscalls_newlib.c \
system_stm32f4xx.c \
main.c \
stm32f4xx_it.c \
usb_eem/usb_bsp.c \
usb_eem/usbd_desc.c \
usb_eem/usbd_usr.c \
usb_eem/usbd_cdc_core.c \
usb_eem/usbd_cdc_vcp.c \
usb_eem/usbd_core.c \
usb_eem/usbd_ioreq.c \
usb_eem/usbd_req.c \
usb_eem/usb_dcd.c \
usb_eem/usb_dcd_int.c \
usb_eem/usb_core.c \
std_io.c \
eth_hal_nxp_ngx.c \
config.c \
crc.c \
ethernetif.c \
third_party/lwip-1.4.0/src/netif/etharp.c \
third_party/lwip-1.4.0/src/core/timers.c \
third_party/lwip-1.4.0/src/netconf.c \
third_party/lwip-1.4.0/src/api/err.c \
third_party/lwip-1.4.0/src/netbuf.c \
third_party/lwip-1.4.0/src/sockets.c \
third_party/lwip-1.4.0/src/tcpip.c \
third_party/lwip-1.4.0/src/core/ipv4/icmp.c \
third_party/lwip-1.4.0/src/inet.c \
third_party/lwip-1.4.0/src/inet_chksum.c \
third_party/lwip-1.4.0/src/ip.c \
third_party/lwip-1.4.0/src/ip_addr.c \
third_party/lwip-1.4.0/src/ip_frag.c \
third_party/lwip-1.4.0/src/dhcp.c \
third_party/lwip-1.4.0/src/mem.c \
third_party/lwip-1.4.0/src/memp.c \
third_party/lwip-1.4.0/src/netif.c \
third_party/lwip-1.4.0/src/pbuf.c \
third_party/lwip-1.4.0/src/raw.c \
third_party/lwip-1.4.0/src/stats.c \
third_party/lwip-1.4.0/src/sys.c \
third_party/lwip-1.4.0/src/tcp.c \
third_party/lwip-1.4.0/src/tcp_in.c \
third_party/lwip-1.4.0/src/tcp_out.c \
third_party/lwip-1.4.0/src/udp.c \
third_party/lwip-1.4.0/src/def.c \
telnet.c \
netio.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c \


#######################################
# binaries
#######################################
GCC_PATH = ./arm-toolchain/bin
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
DUMP = $(GCC_PATH)/$(PREFIX)objdump
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
DUMP = $(PREFIX)objdump
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu with double-precision floating point
FPU = -mfpu=fpv5-d16

# float-abi, hardware floating point
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
#-DUSE_HAL_DRIVER \
#-DSTM32H743xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I../inc \
-I./ \
-ISTM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/ \
-ISTM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/CMSIS/Include/ \
-ISTM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/CMSIS/Device/ST/STM32F4xx/Include \
-ISTM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/STM32_USB_OTG_Driver/inc/ \
-Ithird_party/lwip-1.4.0/port/STM32F4x7/ \
-Ithird_party/lwip-1.4.0/src/include/ \
-Ithird_party/lwip-1.4.0/src/include/ipv4 \
-Iusb_eem \
-Ithird_party/lwip-1.4.0/port/Standalone/httpserver/inc \


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=c99

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=c99 -fpack-struct=4

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = main.ld

# libraries
LIBS = -lc -lm

LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	cp $(BUILD_DIR)/tuner.bin /tmp/tuner.bin
	$(DUMP) -Szh --show-raw-insn $(BUILD_DIR)/tuner.elf >$(BUILD_DIR)/tuner.lst

	
$(BUILD_DIR):
	mkdir $@		


#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)


