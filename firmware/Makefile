######################################
# target
######################################
TARGET = sdr_h7


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
liquid_arm32/freqdem_cfrf_arm32.c \
liquid_arm32/freqmod_arm32.c \
liquid_arm32/firhilb_arm32.c \
liquid_arm32/ampmodem_arm32.c \
liquid_arm32/symsync_rf_arm32.c \
liquid_arm32/firpfb_rf_arm32.c \
liquid_arm32/firpfb_cf_arm32.c \
liquid_arm32/firdes_arm32.c \
liquid_arm32/rrcos_arm32.c \
liquid_arm32/iirfiltsos_rrrf_arm32.c \
liquid_arm32/window_rf_arm32.c \
liquid_arm32/window_cf_arm32.c \
liquid_arm32/dotprod_cf_arm32.c \
liquid_arm32/dotprod_rf_arm32.c \
liquid_arm32/msb_index_arm32.c \
liquid_arm32/rkaiser_arm32.c \
liquid_arm32/math_arm32.c \
liquid_arm32/window_functions_arm32.c \
liquid_arm32/resamp_cf_arm32.c \
liquid_arm32/resamp2_cf_arm32.c \
liquid_arm32/resamp_rf_arm32.c \
liquid_arm32/firfilt_rf_arm32.c \
liquid_arm32/firfilt_cf_arm32.c \
liquid_arm32/nco_cf_arm32.c \
liquid_arm32/randn_arm32.c \
liquid_arm32/eqlms_rf_arm32.c \
liquid_arm32/eqlms_cf_arm32.c \
liquid_arm32/wdelay_rf_arm32.c \
liquid_arm32/wdelay_cf_arm32.c \
fft.c \
fpga.c \
at86rf215.c \
command.c \
crc32.c \
flash_s08.c \
rle.c \
telnet.c \
channel_filter.c \
mbelib/mbelib_test_main.c \
mbelib/ecc.c \
mbelib/imbe7100x4400.c \
mbelib/imbe7200x4400.c \
mbelib/mbelib.c \
p25/p25_decode.c \
p25/p25_stats.c \
p25/audio_leveler.c \
Src/main.c \
globals.c \
syscalls.c \
Src/std_io.c \
Src/lwip.c \
Src/ethernetif.c \
Src/stm32h7xx_it.c \
Src/stm32h7xx_hal_msp.c \
iperf.c \
rtl_tcp_driver.c \
Drivers/BSP/Components/lan8742/lan8742.c \
Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c \
Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_sin_f32.c \
Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c \
Drivers/CMSIS/DSP/Source/SupportFunctions/arm_float_to_q15.c \
Drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_conj_f32.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_fast_q15.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_init_q15.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_f32.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_init_f32.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_f32.c \
Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_f32.c \
Drivers/CMSIS/DSP/Source/StatisticsFunctions/arm_std_f32.c \
Drivers/CMSIS/DSP/Source/StatisticsFunctions/arm_rms_f32.c \
Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_cmplx_dot_prod_f32.c \
Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_dot_prod_f32.c \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_init_f32.o \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_f32.o \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix8_f32.o \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cmplx_mag_f32.o \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal.o \
Drivers/CMSIS/DSP/Source/TransformFunctions/arm_const_structs.o \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_ll_delayblock.c \
Src/system_stm32h7xx.c \
Middlewares/Third_Party/LwIP/src/core/init.c \
Middlewares/Third_Party/LwIP/src/core/netif.c \
Middlewares/Third_Party/LwIP/src/core/inet_chksum.c \
Middlewares/Third_Party/LwIP/src/core/timeouts.c \
Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c \
Middlewares/Third_Party/LwIP/src/api/api_lib.c \
Middlewares/Third_Party/LwIP/src/api/api_msg.c \
Middlewares/Third_Party/LwIP/src/err.c \
Middlewares/Third_Party/LwIP/src/netbuf.c \
Middlewares/Third_Party/LwIP/src/netdb.c \
Middlewares/Third_Party/LwIP/src/netifapi.c \
Middlewares/Third_Party/LwIP/src/sockets.c \
Middlewares/Third_Party/LwIP/src/tcpip.c \
Middlewares/Third_Party/LwIP/src/autoip.c \
Middlewares/Third_Party/LwIP/src/ipv4/ip4.c \
Middlewares/Third_Party/LwIP/src/netif/ethernet.c \
Middlewares/Third_Party/LwIP/src/ipv4/ip4_addr.c \
Middlewares/Third_Party/LwIP/src/ipv4/ip4_frag.c \
Middlewares/Third_Party/LwIP/src/icmp.c \
Middlewares/Third_Party/LwIP/src/igmp.c \
Middlewares/Third_Party/LwIP/src/ip.c \
Middlewares/Third_Party/LwIP/src/dhcp.c \
Middlewares/Third_Party/LwIP/src/dns.c \
Middlewares/Third_Party/LwIP/src/mem.c \
Middlewares/Third_Party/LwIP/src/memp.c \
Middlewares/Third_Party/LwIP/src/pbuf.c \
Middlewares/Third_Party/LwIP/src/raw.c \
Middlewares/Third_Party/LwIP/src/stats.c \
Middlewares/Third_Party/LwIP/src/sys.c \
Middlewares/Third_Party/LwIP/src/tcp.c \
Middlewares/Third_Party/LwIP/src/tcp_in.c \
Middlewares/Third_Party/LwIP/src/tcp_out.c \
Middlewares/Third_Party/LwIP/src/udp.c \
Middlewares/Third_Party/LwIP/src/def.c

# ASM sources
ASM_SOURCES =  \
startup_stm32h743xx.s


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
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32H743xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IInc \
-I./ \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IMiddlewares/Third_Party/LwIP/src/include \
-IMiddlewares/Third_Party/LwIP/system \
-IDrivers/BSP/Components/lan8742/ \
-IDrivers/CMSIS/Include \
-Imbelib \
-Imbelib/include \
-Iliquid_arm32 \
-Ip25 \
-IDrivers/CMSIS/DSP/Include/


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
LDSCRIPT = stm32h743_link.ld

# libraries
#LIBS = -lc -lnosys -lliquid -lm
#LIBS = -lc -lliquid -lm
LIBS = -lc -lm
#LIBDIR = \
#-Lliquid-dsp/

#use this one if no floating point support in printf is required for reduced code size 
#LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

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
	./build_fpga_image.sh
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	cp $(BUILD_DIR)/sdr_h7.bin /tmp/main.bin
	cp $(BUILD_DIR)/sdr_h7.bin $(BUILD_DIR)/sdr_h7_DFU.bin 
	dfu-suffix -a $(BUILD_DIR)/sdr_h7_DFU.bin -v 0483 -p df11 >/dev/null
	$(DUMP) -Szh --show-raw-insn $(BUILD_DIR)/sdr_h7.elf >$(BUILD_DIR)/sdr_h7.lst

	
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
