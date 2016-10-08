#******************************************************************************
#
# Makefile - Rules for building the application.
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

# Check that given variables are set and all have non-empty values,
# die with an error otherwise.
#
# Params:
#   1. Variable name(s) to test.
#   2. (optional) Error message to print.
check_defined = \
    $(foreach 1,$1,$(__check_defined))
__check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $(value 2), ($(strip $2)))))


ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#Find all .c files in the project. All files are located in the folders (and subfolders of) asf and src
SOURCE_Modules_C := $(call rwildcard,Modules/,*.c)
SOURCE_Modules   := $(call rwildcard,Modules/,*.cpp)
SOURCE_lib := $(call rwildcard,lib/,*.cpp) 

SOURCE_C := $(SOURCE_Modules_C)
SOURCE_DIR_C := $(sort $(dir $(SOURCE_Modules_C)))

SOURCE     := $(SOURCE_Modules) $(SOURCE_lib) 
SOURCE_DIR :=$(sort $(dir $(SOURCE)))

VPATH :=$(SOURCE_DIR) $(SOURCE_DIR_C)

INCLUDE_Modules := $(call rwildcard,Modules/,*.h) $(call rwildcard,Modules/,*.hpp)

INCLUDE_lib := $(call rwildcard,lib/,*.h) $(call rwildcard,lib/,*.hpp)

$(call check_defined, BOOST_ROOT, boost root dir)
$(call check_defined, BOOST_LIB_ROOT, boost compiled libs)

INCLUDE_boost := $(BOOST_ROOT)
INCLUDE     := $(INCLUDE_Modules) $(INCLUDE_lib) 
# $(INCLUDE_boost)
# Get include directories from list of all included .h files.
INCLUDE_DIR := $(sort $(dir $(INCLUDE)))

OBJS_C := $(SOURCE_Modules_C:.c=.o) 
OBJS := $(SOURCE:.cpp=.o)
# all output should be in the "out" folder. 
OBJS := $(notdir $(OBJS))
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

OBJS_C := $(notdir $(OBJS_C))
OBJS_C := $(addprefix $(BUILD_DIR)/,$(OBJS_C))




#
# The command for calling the compiler.
#
#CXX=g++
#CC=gcc

#******************************************************************************
#
# Tell the compiler to include debugging information if the DEBUG environment
# variable is set.
#
#******************************************************************************
#ifdef DEBUG
COMPILER_FLAGS = -g 
#endif

#
# The flags passed to the compiler.
#
# -MD generate dependency files as a side-effect
# -MP add a phony target for each dependency other than the main file. 
# -MT change target of the rule emitted by dependency generation. 
#
COMPILER_FLAGS += -MD -MP -MF "$(@:%.o=%.d)" -MT "$(@:%.o=%.d)" -MT "$(@:%.o=%.o)" -Wall -Wextra -pedantic  #-Wconversion
COMPILER_FLAGS += $(addprefix -I, $(INCLUDE_DIR) ) 

# include boost as -isystem since we can not fix the warnings in the lib.
CXXFLAGS+= -isystem $(INCLUDE_boost) -DBOOST_LOG_DYN_LINK

CXXFLAGS += $(COMPILER_FLAGS) -std=c++11
CFLAGS += $(COMPILER_FLAGS) -std=c11

#LDFLAGS += 

ifdef AUX_LIB
LDLIBS += -L$(AUX_LIB)
endif
LDLIBS +=-L$(BOOST_LIB_ROOT) -lboost_thread -lboost_program_options  -lboost_system -lboost_locale
LDLIBS +=-lboost_log -lboost_filesystem -lboost_log_setup -lboost_regex -lboost_date_time 
LDLIBS += -lreadline  -pthread

ifdef ADDRESS_SANITIZER
LDLIBS +=-fsanitize=address -fno-omit-frame-pointer
COMPILER_FLAGS +=-fsanitize=address -fno-omit-frame-pointer
endif


#
# The default rule, which causes init to be built.
#
all: ${BUILD_DIR} ${BUILD_DIR}/QuadGS

print: 
	@echo "Source Files: "${SOURCE}
	@echo "Include Dir: "${INCLUDE_DIR}
	@echo "Obj_c files: "${OBJS_C}
	@echo "Obj files: "${OBJS}

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

#******************************************************************************
#
# The rule for linking the application.
#
#******************************************************************************
${BUILD_DIR}/QuadGS:$(OBJS) $(OBJS_C) 
	@if [ 'x${VERBOSE}' = x ];                              \
	then                                                 \
		echo "  C++    ${<}";                            \
	else                                                 \
		echo ${CXX} -o $(@) $(OBJS) $(OBJS_C)  $(LDFLAGS) $(LDLIBS) $(LOADLIBS);  		 \
	fi
	@${CXX} -o $(@) $(OBJS) $(OBJS_C) $(LDFLAGS) $(LDLIBS) $(LOADLIBS)

#******************************************************************************
#
# The rule for building the object file from each C++ source file.
#
#******************************************************************************
${BUILD_DIR}/%.o: %.cpp
	@if [ 'x${VERBOSE}' = x ];                              \
	then                                                    \
		echo "  C++    ${<}";                               \
	else                                                    \
		echo ${CXX} ${CXXFLAGS} -o ${@} -c ${<}; \
	fi
	@${CXX} ${CXXFLAGS} -o ${@} -c ${<}

#******************************************************************************
#
# The rule for building the object file from each C source file.
#
#******************************************************************************
${BUILD_DIR}/%.o: %.c
	@if [ 'x${VERBOSE}' = x ];                              \
	then                                                    \
		echo "  C    ${<}";                               \
	else                                                    \
		echo ${CC} ${CFLAGS} -o ${@} -c ${<}; \
	fi
	@${CC} ${CFLAGS} -o ${@} -c ${<}


#
#
# Include the automatically generated dependency files.
#
-include ${wildcard ${BUILD_DIR}/*.d}



