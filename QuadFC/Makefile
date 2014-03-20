#******************************************************************************
#
# Makefile - Rules for building the driver library and examples.
#
# Copyright (c) 2005,2006 Luminary Micro, Inc.  All rights reserved.
#
# Software License Agreement
#
# Luminary Micro, Inc. (LMI) is supplying this software for use solely and
# exclusively on LMI's Stellaris Family of microcontroller products.
#
# The software is owned by LMI and/or its suppliers, and is protected under
# applicable copyright laws.  All rights are reserved.  Any use in violation
# of the foregoing restrictions may subject the user to criminal sanctions
# under applicable laws, as well as to civil liability for the breach of the
# terms and conditions of this license.
#
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
# LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
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


