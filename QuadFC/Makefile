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
BUILD_DIR := out
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
include makedefs



#Find all .c files in the project. All files are located in the folders (and subfolders of) asf and src
SOURCE_ASF := $(call rwildcard,asf/,*.c)
SOURCE_APP := $(call rwildcard,src/,*.c)

SOURCE := $(SOURCE_ASF) $(SOURCE_APP)
SOURCE_DIR :=$(sort $(dir $(SOURCE)))

INCLUDE_ASF := $(call rwildcard,asf/,*.h)
INCLUDE_APP := $(wildcard include/*.h)

INCLUDE := $(INCLUDE_ASF) $(INCLUDE_APP)
# Get include directories from list of all included .h files.
INCLUDE_DIR := $(sort $(dir $(INCLUDE)))





OBJS := $(SOURCE:.c=.o)
OBJS := $(notdir $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

VPATH=src:include:$(SOURCE_DIR)

CFLAGS+= $(addprefix -I, $(INCLUDE_DIR)) -DBOARD=ARDUINO_DUE_X -DUDD_ENABLE -D__SAM3X8E__

#
# The default rule, which causes init to be built.
#
all: ${BUILD_DIR} ${BUILD_DIR}/RTOSDemo.elf
	 
#
# The rule to clean out all the build products
#

clean:
	rm -rf ${BUILD_DIR} ${wildcard *.bin}
#	rm -rf ${OBJS} 
#	rm -rf ${OBJS:.o=.d}
#
# The rule to create the target directory
#
${BUILD_DIR}:
	@mkdir ${BUILD_DIR}

${BUILD_DIR}/RTOSDemo.elf:${OBJS}



SCATTER_RTOSDemo=linker_scripts/sam3x/sam3x8/gcc/flash.ld
ENTRY_RTOSDemo=Reset_Handler

#
#
# Include the automatically generated dependency files.
#
-include ${wildcard ${BUILD_DIR}/*.d} __dummy__


	 
.PHONY : Flash
Flash:
	C:\Program Files\Arduino\hardware\tools\bossac.exe -p, --port=COM11 -U false -e -w -v -b "C:\MLU\QuadFC\QuadFC\out\RTOSDemo.bin" -R


