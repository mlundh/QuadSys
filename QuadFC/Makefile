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
# directory structure. This is due to the vact that make does not
# seem to allow target-specific VPATH.
#------------------------------------------------------------------
.SUFFIXES:

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#Clear CFLAGS and VPATH
CFLAGS := 
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := out
VPATH:=
# Add include path for header files. Third party is special, the files expects all
# dirs to be in include path.
INCLUDE_ThirdParty := $(sort $(dir $(call rwildcard,ThirdParty/,*.h)))
CFLAGS+= -I Modules -I HAL -I Top -I $(ROOT_DIR) $(addprefix -I, $(INCLUDE_ThirdParty))

include $(ROOT_DIR)/executables.mk

all: $(EXECUTABLES)

# Include the makefiles that has the defined rules and flags.
INCLUDE_MAKEFILES := $(ROOT_DIR)/rules.mk $(ROOT_DIR)/flags.mk 
include $(INCLUDE_MAKEFILES)

# Second expansion allows us to use $@ and such variables in the prerequisit list.
.SECONDEXPANSION:

# Add categories here if needed.
CATEGORIES := Modules PortLayer Top Test HAL
CATEGORIES_R := ThirdParty
ALL_CATEGORIES := $(CATEGORIES) $(CATEGORIES_R)


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
# Set up all the variables used by each recursive category. 
#
# Variables set:
# $(Category)_SOURCE_DIR
# and adds the source dir to VPATH
#*************************************************************************
$(foreach category,$(CATEGORIES_R),$(eval $(call VARIABLES_R,$(category))))
#$(foreach category,$(CATEGORIES_R),$(info $(call VARIABLES_R,$(category))))


#*************************************************************************
#
# Creates the rule for building the archive library for each sub module
# to each category. This rule looks for source prerequisits in the 
# "src" folder of the sub module.
#
#*************************************************************************
$(foreach category,$(CATEGORIES),\
  $(eval $(call LIB_RULE,$$($(category)_LIB_LIST),$$($(category)_BUILD_DIR))))
#$(foreach category,$(CATEGORIES),\
  $(info $(call LIB_RULE,$$($(category)_LIB_LIST),$$($(category)_BUILD_DIR))))

#*************************************************************************
#
# Creates the rule for building the archive library for each sub module
# to each category. This rule looks for source files recusively in the folder
#
#*************************************************************************
$(foreach category,$(CATEGORIES_R),\
  $(eval $(call LIB_RULE_RECURSIVE,$$($(category)_LIB_LIST),$$($(category)_BUILD_DIR))))
#$(foreach category,$(CATEGORIES_R),\
  $(info $(call LIB_RULE_RECURSIVE,$$($(category)_LIB_LIST),$$($(category)_BUILD_DIR))))

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
# sub modules of each category. This rule is used for recursively
# found sources. 
#
#*************************************************************************
$(foreach category,$(CATEGORIES_R), $(eval $(call O_RULE_recursive, $(BUILD_DIR),$(category))))
#$(foreach category,$(CATEGORIES_R), $(info $(call O_RULE_recursive, $(BUILD_DIR),$(category))))

#*************************************************************************
#
# Creates the rule for linking an application. All applications are
# defined in the variable EXECUTABLES
#
#*************************************************************************
$(foreach exec,$(EXECUTABLES),$(eval $(call ELF_RULE,$(exec), $(BUILD_DIR))))
#$(foreach exec,$(EXECUTABLES),$(info $(call ELF_RULE,$(exec))))

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
	@echo "_SOURCE_DIR:" $(ThirdParty_SOURCE_DIR)
	@echo "VPATH = $(VPATH)"
	
.PHONY : flash
flash:
	python connect1200.py
	bossac -p, --port=ttyACM0 -e -w -v -b "$(ROOT_DIR)/${BUILD_DIR}/QuadFC.bin" -R


