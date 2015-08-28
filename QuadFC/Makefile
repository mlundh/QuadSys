#******************************************************************************
#
# Makefile - Rules for building the driver library and examples.
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
# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
BUILD_DIR := out




ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#Find all .c files in the project. All files are located in the folders (and subfolders of) asf and src
SOURCE_Modules   := $(call rwildcard,Modules/,*.c)
SOURCE_Utilities := $(call rwildcard,Utilities/,*.c)
SOURCE_PortLayer := $(call rwildcard,PortLayer/,*.c)

SOURCE     := $(SOURCE_Modules) $(SOURCE_Utilities) $(SOURCE_PortLayer)
SOURCE_DIR :=$(sort $(dir $(SOURCE)))

INCLUDE_Modules   := $(call rwildcard,Modules/,*.h)
INCLUDE_Utilities := $(call rwildcard,Utilities/,*.h)
INCLUDE_PortLayer := $(call rwildcard,PortLayer/,*.h)
INCLUDE_HAL := $(call rwildcard,HAL/,*.h)


INCLUDE     := $(INCLUDE_Modules) $(INCLUDE_Utilities) $(INCLUDE_PortLayer) $(INCLUDE_HAL)
# Get include directories from list of all included .h files.
INCLUDE_DIR := $(sort $(dir $(INCLUDE)))


OBJS := $(SOURCE:.c=.o)
# all output should be in the "out" folder. 
OBJS := $(notdir $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

VPATH :=$(SOURCE_DIR)

include makedefs

CFLAGS+= $(addprefix -I, $(INCLUDE_DIR)) -DBOARD=ARDUINO_DUE_X -DUDD_ENABLE -D__SAM3X8E__

SCATTER_QuadFC:=PortLayer/asf/sam/utils/linker_scripts/sam3x/sam3x8/gcc/flash.ld
ENTRY_QuadFC:=Reset_Handler

#
# The default rule, which causes init to be built.
#
all: ${BUILD_DIR} ${BUILD_DIR}/QuadFC.elf
#
# The rule to clean out all the build products
# Simple solution, remove the out dir...
#
.PHONY : clean
clean:
	rm -rf ${BUILD_DIR}


#
# The rule to create the target directory
#
${BUILD_DIR}:
	@mkdir ${BUILD_DIR}
	@echo "mkdir	${BUILD_DIR}"

#
#.objs has an implicit rule defined in makedefs
#

${BUILD_DIR}/QuadFC.elf:${OBJS}
#
#
# Include the automatically generated dependency files.
#
-include ${wildcard ${BUILD_DIR}/*.d}



.PHONY : flash
flash:
	python connect1200.py
	bossac -p, --port=ttyACM0 -e -w -v -b "$(ROOT_DIR)/${BUILD_DIR}/QuadFC.bin" -R


