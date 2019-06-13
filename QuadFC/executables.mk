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

EXECUTABLES := QuadFC Test_Modules1 Test_Modules2 Test_Board Test_Utilities Test_Components1

QuadFC_DEPS:= Top/QuadFC HAL/QuadFC
QuadFC_DEPS+= $(addprefix Modules/, Communication FlightController HMI  EventHandler)
QuadFC_DEPS+= $(addprefix Modules/, StateEstimator Utilities SetpointHandler FlightModeHandler Log LogMemBackend)
QuadFC_DEPS+= $(addprefix Components/, Log LogMemBackend PidController Parameters)
QuadFC_DEPS+= $(addprefix PortLayer/, ArduinoDueBoard SpectrumSatellite HMI CY15B104Q_SX_spi_fram)
QuadFC_DEPS+= $(addprefix PortLayer/, MB85RC_i2c_fram Sensors)
QuadFC_DEPS+= $(addprefix ThirdParty/, asf freertos asf_freertos)

Test_Utilities_DEPS:= Top/Test_Utilities PortLayer/TestArduinoDue
Test_Utilities_DEPS+= $(addprefix Test/,TestFW Math Utilities)
Test_Utilities_DEPS+= $(addprefix Modules/, Utilities )
Test_Utilities_DEPS+= $(addprefix ThirdParty/, asf freertos)

Test_Modules1_DEPS:= Top/Test_Modules1 HAL/QuadFC 
Test_Modules1_DEPS+= $(addprefix Test/,TestFW DummyI2C SignalProcessing EventHandler Log LogTestBackend FakeMemory)
Test_Modules1_DEPS+= $(addprefix Components/, Log Parameters SLIP)
Test_Modules1_DEPS+= $(addprefix Modules/, Utilities  EventHandler StateEstimator)
Test_Modules1_DEPS+= $(addprefix Modules/, FlightController Messages MsgBase)
Test_Modules1_DEPS+= $(addprefix PortLayer/, HMI TestArduinoDue SpectrumSatellite Sensors)
Test_Modules1_DEPS+= $(addprefix ThirdParty/, asf freertos)

Test_Modules2_DEPS:= Top/Test_Modules2 HAL/QuadFC 
Test_Modules2_DEPS+= $(addprefix Test/,TestFW  FakeMemory Messages)
Test_Modules2_DEPS+= $(addprefix Components/, SLIP)
Test_Modules2_DEPS+= $(addprefix Modules/, Utilities Messages MsgBase)
Test_Modules2_DEPS+= $(addprefix PortLayer/, TestArduinoDue)
Test_Modules2_DEPS+= $(addprefix ThirdParty/, asf freertos)

Test_Board_DEPS:= Top/Test_Board HAL/QuadFC
Test_Board_DEPS+= $(addprefix Test/, ArduinoDueBoard TestFW )
Test_Board_DEPS+= $(addprefix Modules/, Utilities )
Test_Board_DEPS+= $(addprefix Components/, Log LogMemBackend)
Test_Board_DEPS+= $(addprefix PortLayer/, TestArduinoDue ArduinoDueBoard CY15B104Q_SX_spi_fram MB85RC_i2c_fram)
Test_Board_DEPS+= $(addprefix ThirdParty/, asf freertos asf_freertos)

Test_Components1_DEPS:= Top/Test_Components1 HAL/QuadFC
Test_Components1_DEPS+= $(addprefix ThirdParty/, asf freertos asf_freertos)
Test_Components1_DEPS+= $(addprefix PortLayer/, TestArduinoDue )
Test_Components1_DEPS+= $(addprefix Components/, Parameters SLIP)
Test_Components1_DEPS+= $(addprefix Test/,TestFW Parameters FakeMemory)
Test_Components1_DEPS+= $(addprefix Modules/, Messages MsgBase EventHandler Utilities)