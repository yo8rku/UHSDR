# Author: Harald Baumgart DL4SAI
# everybody may  copy, use or modify this file
# there is no guarantee for anything by the author
#
# HB 20.8.2015
# make (all)  -> compile the arm-executable .bin and .elf, generate .map and .dmp
# make clean  -> remove the executable files, map, dmp and all object files (.o)
# make handy  -> rm all .o (but not executables, .map and .dmp)
#
# Rev. 09/08/2015 corrected by Andreas Richter DF8OE 



# set these environent to your individual values
# This doesn't work as the prefix and standard locations vary from compiler to compiler
# and of course the owners preferences of where to install these packages
#
PRJ  = mchf
LPRJ = mchf
#PREFIX = /usr
#GCC_LIB_VERSION = 4.8

CROSS_COMPILE ?= arm-none-eabi-

# This is the flash memory address where to place the program. If you compile for 
# bootloader operation, use 0x08010000, else 0x08000000.
BURN_ADDR := 0x08010000

# BASE := /opt/gcc-arm-none-eabi-4_9-2015q1
# LIBS := $(BASE)/lib/gcc/arm-none-eabi/
# /opt/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-

# CPU compiler options
CFLAGS := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
CFLAGS += -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -DUSE_STDPERIPH_DRIVER  
# Optimizer options
CFLAGS += -ffunction-sections -O0

# Linker flags
LDFLAGS := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb

# Libraries to include
LIBS := -Llibs -larm_cortexM4lf_math 
LIBS += -lm -lg -lc -lgcc
#LIBS += -L$(PREFIX)/arm-none-eabi/lib/armv7e-m/fpu
# LIBS += -L$(PREFIX)/lib/gcc/arm-none-eabi/$(GCC_LIB_VERSION)/armv7e-m/fpu

#
# Directories having include files separated by function
#
DIRS_PROJECT := \
	drivers/cat \
	drivers/cat/usb \
	drivers/audio \
	drivers/audio/cw \
	drivers/audio/codec \
	drivers/audio/softdds \
	drivers/audio/filters \
	drivers/keyboard \
	drivers/keyboard/usb \
	drivers/ui/oscillator \
	drivers/ui \
	drivers/ui/misc \
	drivers/ui/encoder \
	drivers/ui/lcd \
	hardware \
	misc \
	misc/v_eprom

DIRS_STM_USB := \
	usb/usbh/core/inc \
	usb/usbh/class/HID/inc \
	usb/otg/inc \
	usb \
	usb/usbd/core/inc \
	usb/usbd/class/audio/inc \
	usb/usbd/class/cdc/inc \

DIRS_ARM_CMSIS := \
	cmsis \
	cmsis_boot \
	cmsis_lib/include \
	cmsis_lib/source \

# Combine all include files related directories to a big search path
SUBDIRS := $(DIRS_ARM_CMSIS) $(DIRS_STM_USB) $(DIRS_PROJECT)

#
# Every source-file has to be mentioned here 
#
SRCS_PROGRAM := \
	main.c \
	drivers/cat/cat_driver.c \
	drivers/cat/usb/usbd_usr.c \
	drivers/cat/usb/usbd_bsp.c \
	drivers/cat/usb/usbd_desc.c \
	drivers/cat/usb/usbd_cdc_vcp.c \
	drivers/audio/cw/cw_gen.c \
	drivers/audio/audio_driver.c \
	drivers/audio/codec/codec.c \
	drivers/audio/codec/i2s.c \
	drivers/audio/softdds/softdds.c \
	drivers/keyboard/usb/usbh_bsp.c \
	drivers/keyboard/usb/usbh_usr.c \
	drivers/keyboard/keyb_driver.c \
	drivers/ui/oscillator/si570.c \
	drivers/ui/oscillator/mcp9801.c \
	drivers/ui/oscillator/lo.c \
	drivers/ui/ui_menu.c \
	drivers/ui/ui_driver.c \
	drivers/ui/encoder/ui_rotary.c \
	drivers/ui/lcd/ui_lcd_hy28.c \
	drivers/ui/lcd/ui_lcd_hy28_fonts.c \
	hardware/mchf_hw_i2c2.c \
	hardware/mchf_hw_i2c.c \
	hardware/mchf_board.c \
	syscalls/syscalls.c \
	misc/v_eprom/eeprom.c \
	stdio/printf.c
	
SRC_STM_USB := \
	usb/usbh/core/src/usbh_hcs.c \
	usb/usbh/core/src/usbh_core.c \
	usb/usbh/core/src/usbh_ioreq.c \
	usb/usbh/core/src/usbh_stdreq.c \
	usb/usbh/class/HID/src/usbh_hid_keybd.c \
	usb/usbh/class/HID/src/usbh_hid_core.c \
	usb/usbh/class/HID/src/usbh_hid_mouse.c \
	usb/otg/src/usb_core.c \
	usb/otg/src/usb_hcd_int.c \
	usb/otg/src/usb_dcd.c \
	usb/otg/src/usb_dcd_int.c \
	usb/otg/src/usb_otg.c \
	usb/otg/src/usb_hcd.c \
	usb/usbd/core/src/usbd_core.c \
	usb/usbd/core/src/usbd_ioreq.c \
	usb/usbd/core/src/usbd_req.c \
	usb/usbd/class/audio/src/usbd_audio_core.c \
	usb/usbd/class/audio/src/usbd_audio_out_if.c \
	usb/usbd/class/cdc/src/usbd_cdc_core.c \

# These are probably located totally wrong...
SRC_STM32 := \
	cmsis_lib/source/stm32f4xx_usart.c \
	cmsis_lib/source/misc.c \
	cmsis_lib/source/stm32f4xx_pwr.c \
	cmsis_lib/source/stm32f4xx_syscfg.c \
	cmsis_lib/source/stm32f4xx_gpio.c \
	cmsis_lib/source/stm32f4xx_wwdg.c \
	cmsis_lib/source/stm32f4xx_adc.c \
	cmsis_lib/source/stm32f4xx_exti.c \
	cmsis_lib/source/stm32f4xx_i2c.c \
	cmsis_lib/source/stm32f4xx_dac.c \
	cmsis_lib/source/stm32f4xx_fsmc.c \
	cmsis_lib/source/stm32f4xx_flash.c \
	cmsis_lib/source/stm32f4xx_spi.c \
	cmsis_lib/source/stm32f4xx_dma.c \
	cmsis_lib/source/stm32f4xx_rtc.c \
	cmsis_lib/source/stm32f4xx_tim.c \
	cmsis_lib/source/stm32f4xx_rcc.c \

SRC_ARM_CMSIS := \
	cmsis_boot/system_stm32f4xx.c \
	cmsis_boot/startup/startup_stm32f4xx.c \

# List all sources for later compilation
SRC := $(SRC_ARM_CMSIS) $(SRC_STM32) $(SRC_STM_USB) $(SRCS_PROGRAM) 

# ------------- nothing to change below this line ---------------------- 


INC_DIRS = $(foreach d, $(SUBDIRS), -I$d)

CC = $(CROSS_COMPILE)gcc
OC = $(CROSS_COMPILE)objcopy
ECHO = echo

#
# Build in programmingh function
#
BURN_PROG := openocd
BURN_DEV  := -f board/stm32f4discovery.cfg
BURN_INIT := -c "reset init"
BURN_CMD  := -c "flash write_image erase $(PRJ).bin $(BURN_ADDR) bin"
BURN_EXIT := -c "reset run" -c "shutdown"

ifdef SystemRoot # case WINxx:
   RM = del /Q
   FixPath = $(subst /,\,$1)
else             # case Linux:
   ifeq ($(shell uname), Linux)
      RM = rm -f
      FixPath = $1
   endif
endif

# how to compile individual object files
OBJS  :=
OBJS  += $(SRC:.c=.o)
.c.o:
	@$(ECHO) "  [CC] $@" 
	@$(CC) $(CFLAGS) -c ${INC_DIRS} $< -o $@

.PHONY: all clean program

all: $(LPRJ).elf $(PRJ).bin

# compilation
$(LPRJ).elf: $(OBJS) $(SRC)
	@$(ECHO) "  [ELF] ${LPRJ}.elf"
	@$(CC) $(LDFLAGS) -Tarm-gcc-link.ld -Xlinker --gc-sections -Llibs -Wl,-Map,${LPRJ}.map -o${LPRJ}.elf $(OBJS) $(LIBS)

$(PRJ).bin: $(LPRJ).elf
	@$(ECHO) "  [BIN] $(PRJ).bin"
	@$(OC) -v -O binary $< $(PRJ).bin

# cleaning rule
clean:
	$(RM) $(call FixPath,$(OBJS))
	$(RM) $(call FixPath,$(LPRJ).elf)
	$(RM) $(call FixPath,$(PRJ).bin)
	$(RM) $(call FixPath,$(LPRJ).map)
	$(RM) $(call FixPath,*~)

handy:
	$(RM) $(call FixPath,$(OBJS))
	$(RM) $(call FixPath,*~)

program:
	$(BURN_PROG) $(BURN_DEV) $(BURN_INIT) $(BURN_CMD) $(BURN_EXIT)	

# DO NOT DELETE THIS LINE -- make depend depends on it.

