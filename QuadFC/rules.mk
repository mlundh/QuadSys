#******************************************************************************
#
# rules.mk - the build rules.
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

#******************************************************************************
#
# The function for defining all variables needed by each category.
# $1 = the category
#
#******************************************************************************
define VARIABLES
$(1)_BUILD_DIR :=$$(BUILD_DIR)/$(1)
$(1)_LIST := $$(patsubst %/,%,$$(subst $$(ROOT_DIR)/,,$$(sort $$(dir $$(wildcard $$(ROOT_DIR)/$(1)/*/)))))
$(1)_LIB_LIST := $$(addprefix lib, $$(addsuffix .a, $$(notdir $$($(1)_LIST))))

# Add to phony and precious, we want to keep the build output, even if 
# it is intermediate.
.PHONY: $$($(1)_LIST)
.PRECIOUS : ${$(1)_BUILD_DIR}/%.o $($(1)_BUILD_DIR)/%.a

endef

#******************************************************************************
#
# The function creating a phony target for each sub-module
# $1 = build dir. example out
# $2 = dir of current category example Modules/parameters
#
#******************************************************************************
define OBJFILES
$(2)_OBJS:= $$(addprefix $(1)/$(2)/, $$(patsubst %.c, %.o ,$$(notdir $$(call rwildcard,$(2)/src,*.c))))
$(2)_OBJS+= $$(addprefix $(1)/$(2)/, $$(patsubst %.s, %.o ,$$(notdir $$(call rwildcard,$(2)/src,*.s))))
endef

#******************************************************************************
#
# The function for defining all variables needed by each category that is
# searched recursively. 
# $1 = the category
#
#******************************************************************************
define VARIABLES_R
$(1)_SOURCE_DIR := $$(patsubst %/,%, $$(sort $$(dir $$(call rwildcard,$(1)/,*.c))))
$(1)_SOURCE_DIR += $$(patsubst %/,%, $$(sort $$(dir $$(call rwildcard,$(1)/,*.s))))
$(1)_OBJS := $$(addprefix $(BUILD_DIR)/$(1)/, $$(patsubst %.c, %.o, $$(notdir $$(call rwildcard,$(1)/,*.c))))
$(1)_OBJS += $$(addprefix $(BUILD_DIR)/$(1)/, $$(patsubst %.s, %.o, $$(notdir $$(call rwildcard,$(1)/,*.s))))

# Add source to vpath.
VPATH += $$($(1)_SOURCE_DIR)
endef

#******************************************************************************
#
# The rule for building the object file from each C source file. It also creates
# a rule for the output directory of the current module.
# Example: out/Modules/parameters/parameters.o depends on 
#          Modules/parameters/src/parameters.c 
# $1 = build dir. example out
# $2 = dir of current category example Modules/parameters
#
#******************************************************************************
define O_RULE

$(addprefix $(1)/,$(2)):
	@mkdir -p $${@}
	@echo "mkdir  $${@}"

$(addprefix $(1)/,$(2)/%.o) : $(2)/src/%.c | $(addprefix $(1)/,$(2))
	@if [ ! -z $V ] && [ $V -eq 1 ];   \
	 then                              \
	     set -x;                       \
	 else                              \
	     echo "  CC    $${<}";           \
	 fi;                               \
	$${CC} ${CFLAGS} -o $${@} -c $${<};
endef

#******************************************************************************
#
# The rule for building the object file from each C source file. It also creates
# a rule for the output directory of the current module.
# Example: out/Modules/parameters/parameters.o depends on 
#          Modules/parameters/src/parameters.c 
# $1 = build dir. example out
# $2 = dir of current category example Modules
#
#******************************************************************************
define O_RULE_recursive

$(addprefix $(1)/,$(2)):
	@mkdir -p $${@}
	@echo "mkdir  $${@}"

$(addprefix $(1)/,$(2)/%.o) : %.s | $(addprefix $(1)/,$(2))
	@if [ ! -z $V ] && [ $V -eq 1 ];   \
	 then                              \
	     set -x;                       \
	 else                              \
	     echo "  CC    $${<}";           \
	 fi;                               \
	$${CC} ${CFLAGS} -o $${@} -c $${<};


$(addprefix $(1)/,$(2)/%.o) : %.c | $(addprefix $(1)/,$(2))
	@if [ ! -z $V ] && [ $V -eq 1 ];   \
	 then                              \
	     set -x;                       \
	 else                              \
	     echo "  CC    $${<}";           \
	 fi;                               \
	$${CC} ${CFLAGS} -o $${@} -c $${<};

endef


#******************************************************************************
#
# The rule for determining all objects that are required for linking the application.
#
# $1 = Name of application
# $2 = build dir, example bin
#******************************************************************************
define EXECOBJ
$(1)_OBJS += $($(2)_OBJS)
endef


#******************************************************************************
#
# The rule for linking the application. Maps folder names to libs.
# Example: QuadFC depends on out/Modules/libParameters.a out/Modules/libCommunication.a
#
# $1 = Name of application
# $2 = build dir, example bin
#******************************************************************************
define ELF_RULE
.PHONY:$(1)

# Map name to elf in output.
$(1):${BUILD_DIR}/$(1).elf
# Create a variable that contains the name of the .bin file
${BUILD_DIR}/$(1).elf: BIN=$$(patsubst %.elf, %.bin, $$(@))
${BUILD_DIR}/$(1).elf: $($(1)_OBJS)| $${BUILD_DIR}
	@if [ ! -z $V ] && [ $V -eq 1 ]; \
	 then                            \
	     set -x;                     \
	 else                            \
	     echo "  LD    $${@}";       \
	 fi;                             \
	$${CC} -o $$(@) $$(^) $$(LDFLAGS); \
	$${OBJCOPY} -O binary $${@} $$(BIN)
endef


#******************************************************************************
#
# The rule for including all automatically generated dep files. 
#
# $1 = Name of application
# $2 = build dir, example bin
#******************************************************************************
define DEP_INC
-include $${wildcard $(addprefix $(1)/,$(2))/*.d}
endef
