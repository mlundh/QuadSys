#******************************************************************************
#
# Makefile - Rules and deps for each platform.
#
# Copyright (C) 2016  Martin Lundh
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

# The variables defining an executable and the parts that are needed to build
# the executable. 
#
# RULES:
# Add the executable to the variable EXECUTABLES and create a variable 
# <nameOfExec>_DEPS. Add all dependencies to this variable.
#

EXECUTABLES := QuadFC TestFC_RTOS TestFC_Utilities

QuadFC_DEPS:= Top/QuadFC HAL/QuadFC
QuadFC_DEPS+= $(addprefix Modules/, Communication FlightController HMI Parameters EventHandler)
QuadFC_DEPS+= $(addprefix Modules/, StateEstimator Utilities SetpointHandler FlightModeHandler)
QuadFC_DEPS+= $(addprefix PortLayer/, Actuators Board Communication HMI Memory Sensors)
QuadFC_DEPS+= $(addprefix ThirdParty/, asf freertos asf_freertos)

TestFC_Utilities_DEPS:= ThirdParty/asf Top/TestFC_Utilities 
TestFC_Utilities_DEPS+= $(addprefix Test/,TestFW Math)
TestFC_Utilities_DEPS+= $(addprefix Modules/, Utilities )

TestFC_RTOS_DEPS:= Top/TestFC_RTOS HAL/QuadFC
TestFC_RTOS_DEPS+= $(addprefix Test/,TestFW DummyI2C SignalProcessing EventHandler)
TestFC_RTOS_DEPS+= $(addprefix Modules/, Utilities StateEstimator Parameters EventHandler FlightController HMI)
TestFC_RTOS_DEPS+= $(addprefix PortLayer/, Sensors Communication HMI)
TestFC_RTOS_DEPS+= $(addprefix ThirdParty/, asf freertos)