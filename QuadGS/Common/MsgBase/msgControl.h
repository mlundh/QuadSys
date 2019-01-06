/*
 * QGS_IoHeader.h
 *
 * Copyright (C) 2017 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CORE_DATAMSG_SRC_QCMSGHEADER_H_
#define CORE_DATAMSG_SRC_QCMSGHEADER_H_

/**
 * @file QCMsgHeaderg.h
 *
 * A QSP is defined as:
 *
 *
 *  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 *  +-------------------------------+
 * 0|            address            |
 *  +-------------------------------+
 * 1| R|         Control            |
 *  +-------------------------------+
 * 2|          payload size H       |
 *  +-------------------------------+
 * 3|          payload size L       |
 *  +-------------------------------+
 *
 *  Where the R field indicates resend.
 *
 *
 *
 */

#include <string>
#include <vector>
#include <memory>

namespace QuadGS {

    enum LogControl
    {
        Name = 0,
        Entry = 1,
        StopAll = 2,
    };
    typedef enum LogControl LogControl_t;


    enum StatusControl
    {
        Cont = 0,
        Ack = 1,
        Nack = 2,
        Error = 3,
        NotAllowed = 4,
        UnexpectedSequence = 5,
        NotValidSlipPacket = 6,
        BufferOverrun = 7,
        NotImplemented = 8,

    };
    typedef enum StatusControl StatusControl_t;

    enum TransmissionControl
    {
        OK = 0,
        NOK = 1,
    };
    typedef enum TransmissionControl TransmissionControl_t;

    enum DebugControl
    {
        GetRuntimeStats = 0,
        SetRuntimeStats = 1,
        GetErrorMessages = 2,

    };
    typedef enum DebugControl DebugControl_t;


	enum ParametersControl
	{
		SetTree = 0,
		GetTree = 1,
		Value = 2,
		Save = 3,
		Load = 4,
	};
	typedef enum ParametersControl ParametersControl_t;

} /* namespace QuadGS */

#endif /* CORE_DATAMSG_SRC_QCMSGHEADER_H_ */
