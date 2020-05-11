#******************************************************************************
#
# flags.mk - flags used by all modules.
#
# Copyright (C) 2014  Martin Lundh
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
#******************************************************************************

#
# The compiler used.
#
CC=arm-none-eabi-gcc

#
# The linker used.
#
LD=arm-none-eabi-gcc
#
# The command for extracting images from the linked executables.
#
OBJCOPY=arm-none-eabi-objcopy

#
# The command for calling the library archiver.
#
AR=arm-none-eabi-ar

AS = $(CC)-x assembler-with-cpp



ifeq ($(PLATFORM),Due)
	CPU := -mcpu=cortex-m3
    MCU := $(CPU) -mthumb 
	DEFINES := -DBOARD=ARDUINO_DUE_X -DUDD_ENABLE -D__SAM3X8E__
	SCATTER_QuadFC:= Drivers/Due/asf/sam/utils/linker_scripts/sam3x/sam3x8/gcc/flash.ld
	ENTRY_QuadFC:=-Wl,--entry=Reset_Handler
else ifeq ($(PLATFORM),Titan)
	CPU := -mcpu=cortex-m4
	FPU := -mfpu=fpv4-sp-d16
	FLOAT-ABI := -mfloat-abi=hard
	MCU := $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
	DEFINES := -DUSE_HAL_DRIVER -DSTM32F413xx -DUSE_HAL_DRIVER
	SCATTER_QuadFC:=Drivers/Titan/STM32F413VGTx_FLASH.ld

else
	$(info Unsupported Platform.)
endif


#
# Entry function etc.
#



#******************************************************************************
#
# Tell the compiler to include debugging information if the DEBUG environment
# variable is set.
#
#******************************************************************************

ifdef DEBUG
DBGFLAGS := -g3 -O0 
else
# optimization breaks the application. Investigate why! 
CFLAGS += -O2
endif

ifdef QUADFC_STATS
CFLAGS += -DQuadFCStats
endif

#
# The flags passed to the compiler.
#
# -MMD generate dependency files as a side-effect
# -MP add a phony target for each dependency other than the main file. 
# -MT change target of the rule emitted by dependency generation. Use multiple calls to
# create multiple targets. Here we add the dependency file as a target.
# -Wmissing-prototypes -Wstrict-prototypes

CFLAGS+= $(MCU) $(DBGFLAGS) $(C_DEFS) -MMD -MP -MT "$$(patsubst %.o, %.d, $$(notdir $$(@)))" -MT "$$(@)"
CFLAGS+= -fdata-sections -ffunction-sections -mlong-calls -Wall -c -std=gnu99 -Wshadow
CFLAGS+= $(DEFINES) 

#
# The flags passed to the linker.
#
LDFLAGS= $(MCU) -specs=nano.specs -Wl,-Map,"$(@).map",--cref -lm -lc -lm -lnosys 
LDFLAGS+= -Wl,--gc-sections -T $(SCATTER_QuadFC) 
LDFLAGS+= $(ENTRY_QuadFC) 
