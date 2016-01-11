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
$(1)_LIB_LIST := $$(addprefix $$($(1)_BUILD_DIR)/,$$(addprefix lib, $$(addsuffix .a, $$(notdir $$($(1)_LIST)))))
$(1)_SOURCE_DIR   := $$(patsubst %/,%, $$(sort $$(dir $$(call rwildcard,$(1)/,*.c))))
# Add source to vpath.
VPATH += $$($(1)_SOURCE_DIR)
# Add to phony and precious, we want to keep the build output, even if 
# it is intermediate.
.PHONY: $$($(1)_LIST)
.PRECIOUS : ${$(1)_BUILD_DIR}/%.o $($(1)_BUILD_DIR)/%.a
# The rule to mapp folder name to lib name.
$$($(1)_LIST): $(1)/% : $$($(1)_BUILD_DIR)/lib%.a 
# The rule to make the output directory.
$$($(1)_BUILD_DIR) :
	@mkdir -p $${$(1)_BUILD_DIR}
	@echo "mkdir  $${$(1)_BUILD_DIR}"
endef

#******************************************************************************
#
# The rule for building the object file from each C source file.
# $1 = build dir for current category.
#
#******************************************************************************
define O_RULE
$(1)/%.o: %.c | $(1)
	@if [ ! -z $V ] && [ $V -eq 1 ];   \
	 then                              \
	     set -x;                       \
	 else                              \
	     echo "  CC    $${<}";           \
	 fi;                               \
	$${CC} ${CFLAGS} -o $${@} -c $${<};
endef


define DEP_INC
-include $${wildcard $$($(1)_BUILD_DIR)/*.d}
endef
#
# The rule for creating an object library.
# $1 = list of libs in current category, example: out/Modules/libParameters.a
# $2 = build dir of current category, example: out/Modules
#******************************************************************************
define LIB_RULE
$(1):$(2)/lib%.a : $$$$(addprefix $(2)/, $$$$(notdir $$$$(subst .c,.o, $$$$(wildcard $$$$(notdir $(2))/%/src/*.c)))) | $${BUILD_DIR}
	@if [ ! -z $V ] && [ $V -eq 1 ];  \
	 then                             \
	     set -x;                      \
	 else                             \
	     echo "  AR    $${@}";        \
	 fi;                              \
	$${AR} -cr $${@} $${^};
endef

#
# The rule for creating an object library and finding the prereq. recrsively
# $1 = list of libs in current category, example: out/Modules/libParameters.a
# $2 = build dir of current category, example: out/Modules
#******************************************************************************
define LIB_RULE_RECURSIVE
$(1): $(2)/lib%.a : $$$$(addprefix $(2)/, $$$$(notdir $$$$(subst .c,.o, $$$$(call rwildcard, $$$$(notdir $(2))/%/,*.c)))) | $${BUILD_DIR}
	@if [ ! -z $V ] && [ $V -eq 1 ];  \
	 then                             \
	     set -x;                      \
	 else                             \
	     echo "  AR    $${@}";        \
	 fi;                              \
	$${AR} -cr $${@} $${^};
endef
#	     echo $${AR} -MMD -cr $${@} $${^}; \
#******************************************************************************
#
# The rule for linking the application. Maps folder names to libs.
#
#******************************************************************************
define ELF_RULE
.PHONY:$(1)
# Map name to elf in output.
$(1):${BUILD_DIR}/$(1).elf
# Create a variable that contains the name of the .bin file
${BUILD_DIR}/$(1).elf: BIN=$$(patsubst %.elf, %.bin, $$(@))
${BUILD_DIR}/$(1).elf: $$$$(addsuffix .a,$$$$(addprefix $${BUILD_DIR}/, $$$$(subst /,/lib,$$$$($(1)_DEPS)))) | $${BUILD_DIR}
	@if [ ! -z $V ] && [ $V -eq 1 ]; \
	 then                            \
	     set -x;                     \
	 else                            \
	     echo "  LD    $${@}";       \
	 fi;                             \
	$${CC} -o $$(@) -Wl,--start-group $$(^)  -Wl,--end-group out/ThirdParty/syscalls.o $$(LDFLAGS); \
	$${OBJCOPY} -O binary $${@} $$(BIN)
endef
