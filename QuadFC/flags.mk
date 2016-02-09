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
# The command for extracting images from the linked executables.
#
OBJCOPY=arm-none-eabi-objcopy

#
# The command for calling the library archiver.
#
AR=arm-none-eabi-ar

#
# Entry function etc.
#

SCATTER_QuadFC:=ThirdParty/asf/sam/utils/linker_scripts/sam3x/sam3x8/gcc/flash.ld
ENTRY_QuadFC:=Reset_Handler


#******************************************************************************
#
# Tell the compiler to include debugging information if the DEBUG environment
# variable is set.
#
#******************************************************************************
ifdef DEBUG
CFLAGS += -g -O0
else
CFLAGS += -O1
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

CFLAGS+= -mthumb -mcpu=cortex-m3 -MMD -MP -MT "$$(patsubst %.o, %.d, $$(notdir $$(@)))" -MT "$$(@)"
CFLAGS+= -fdata-sections -ffunction-sections -mlong-calls -Wall -c -std=gnu99 -Wshadow
CFLAGS+= -DBOARD=ARDUINO_DUE_X -DUDD_ENABLE -D__SAM3X8E__

#
# The flags passed to the linker.
#
LDFLAGS= -mthumb -Wl,-Map,"$(@).map" -Wl,--start-group -lm  -Wl,--end-group 
LDFLAGS+= -Wl,--gc-sections -mcpu=cortex-m3 -T $(SCATTER_QuadFC) -Wl,--cref 
LDFLAGS+= -Wl,--entry=$(ENTRY_QuadFC) -mthumb  -specs=nano.specs

