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

# ---------------------------- Warning!---------------------------
# .c files are not allowed to have the same name anywhere in the 
# directory structure. This is due to the fact that make does not
# seem to allow target-specific VPATH.
#------------------------------------------------------------------


PLATFORMS:= Titan Due

ifndef PLATFORM
        $(error PLATFORM is not set)
endif

ifneq ($(filter $(PLATFORM),$(PLATFORMS)),)
    $(info Building $(PLATFORM))
else
    $(error $(PLATFORM) is not supported, only values in the following list are supported: "$(PLATFORMS)")
endif


.SUFFIXES:

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#Clear CFLAGS and VPATH
CFLAGS := 
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := out/$(PLATFORM)
VPATH:=

# Add categories here if needed.
CATEGORIES := Modules HwComponents Top Test HAL BaseComponents Boards/$(PLATFORM) Components 
CATEGORIES_R_PLF := Drivers OS
CATEGORIES_R :=  

#Combine categories to ensure coorrect targets and vpath.
CATEGORIES_R_PLATFORM := $(addsuffix /$(PLATFORM),$(CATEGORIES_R_PLF)) $(CATEGORIES_R)
ALL_CATEGORIES := $(CATEGORIES) $(CATEGORIES_R_PLF) $(CATEGORIES_R) 
CATEGORIES_R += $(CATEGORIES_R_PLF)


#$(error CATEGORIES_R $(CATEGORIES_R_PLATFORM))
# Add include path for header files. Third party code is special, the files expects all
# dirs to be in include path.

I_ThirdParty := $(sort $(dir $(call rwildcard,Drivers/$(PLATFORM),*.h)))
I_ThirdParty += $(sort $(dir $(call rwildcard,OS/$(PLATFORM),*.h)))
INCLUDE_ThirdParty := $(addprefix -I, $(I_ThirdParty))

INCLUDE_PATH := $(addprefix -I, $(CATEGORIES) Boards/$(PLATFORM)/Config/inc)

CFLAGS+= $(INCLUDE_PATH) -I $(ROOT_DIR) $(INCLUDE_ThirdParty)

include $(ROOT_DIR)/executables.mk

#Create all targets
all: $(EXECUTABLES)


# Include the makefiles that has the defined rules and flags.
INCLUDE_MAKEFILES := $(ROOT_DIR)/rules.mk $(ROOT_DIR)/flags.mk 
include $(INCLUDE_MAKEFILES)

# Second expansion allows us to use $@ and such variables in the prerequisit list.
.SECONDEXPANSION:




#*************************************************************************
# Set up all the variables used by each category. Also sets up a phony target that 
# lists all sub-modules of each category, and a rule to make the output dir of the
# category.
#
# Variables set:
# $(Category)_BUILD_DIR
# $(Category)_LIST
# $(Category)_LIB_LIST
# and also sets up the target $(Category)_LIST
#
#*************************************************************************
$(foreach category,$(ALL_CATEGORIES),$(eval $(call VARIABLES,$(category))))
#$(foreach category,$(ALL_CATEGORIES),$(info $(call VARIABLES,$(category))))

#*************************************************************************
#
# Sets up the list of all obj files for each module.
#
# Variables set:
# $(sub_category)_OBJS 
#*************************************************************************
$(foreach category,$(CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(eval $(call OBJFILES, $(BUILD_DIR),$(sub_category)))))
#$(foreach category,$(CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(info $(call OBJFILES, $(BUILD_DIR),$(sub_category)))))


#*************************************************************************
# Set up all the variables used by each recursive category. 
#
# Variables set:
# $(Category)_SOURCE_DIR
# $(Category)_OBJS 
# and adds the source dir to VPATH
#*************************************************************************
$(foreach category,$(CATEGORIES_R_PLATFORM),$(eval $(call VARIABLES_R,$(category))))
#$(foreach category,$(CATEGORIES_R_PLATFORM),$(info $(call VARIABLES_R,$(category))))

#*************************************************************************
#
# Creates the rule for building .o file from each source file in the 
# sub modules of each category.
#
#*************************************************************************
$(foreach category,$(CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(eval $(call O_RULE, $(BUILD_DIR),$(sub_category)))))
#$(foreach category,$(CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(info $(call O_RULE, $(BUILD_DIR),$(sub_category)))))

#*************************************************************************
#
# Creates the rule for building .o file from each source file in the 
# sub modules of each recursive category. \
#
#*************************************************************************
$(foreach category,$(CATEGORIES_R_PLATFORM), $(eval $(call O_RULE_recursive, $(BUILD_DIR),$(category))))
#$(foreach category,$(CATEGORIES_R), $(info $(call O_RULE_recursive, $(BUILD_DIR),$(category))))

#*************************************************************************
#
# Creates the rule for creating the list of all objects an excecutable  needs. 
#
#*************************************************************************
$(foreach excecutable, $(EXECUTABLES),\
  $(foreach dep, $($(excecutable)_DEPS),\
  $(eval $(call EXECOBJ, $(excecutable),$(dep)))))
#$(foreach excecutable, $(EXECUTABLES),\
  $(foreach dep, $($(excecutable)_DEPS),\
  $(eval $(call EXECOBJ, $(excecutable),$(dep)))))

#*************************************************************************
#
# Creates the rule for linking an application. All applications are
# defined in the variable EXECUTABLES
#
#*************************************************************************
$(foreach exec,$(EXECUTABLES),$(eval $(call ELF_RULE,$(exec), $(BUILD_DIR))))
#$(foreach exec,$(EXECUTABLES),$(info $(call ELF_RULE,$(exec), $(BUILD_DIR))))


#*************************************************************************
#
# Include the automatically generated dep files.
#
#*************************************************************************
$(foreach category,$(ALL_CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(eval $(call DEP_INC, $(BUILD_DIR),$(sub_category)))))
#$(foreach category,$(CATEGORIES),\
  $(foreach sub_category,$($(category)_LIST),\
  $(info $(call DEP_INC, $(BUILD_DIR),$(sub_category)))))

.PHONY: all clean print

$(BUILD_DIR):
	@mkdir -p ${BUILD_DIR}
	@echo "mkdir ${BUILD_DIR}"
#
# The rule to clean out all the build products
# Simple solution, remove the out dir...
#
clean:
	rm -rf ${BUILD_DIR}

print:
	@echo "BUILD_DIR:" $(ThirdParty_BUILD_DIR)
	@echo "TOP:" $(Scott-E_DEPS)


