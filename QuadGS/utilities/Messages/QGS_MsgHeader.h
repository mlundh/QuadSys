/*
 * Parameters.cpp
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

#include "QGS_Msg.h"

namespace QuadGS {


class QGS_MsgHeader: public QGS_Msg
{
public:

    typedef std::shared_ptr<QGS_MsgHeader> ptr;

    enum addresses
    {
        Parameters = 0,
        Log = 1,
        State = 2,
        Status = 3,
        Debug = 4,
        Transmission = 5,
    };
    typedef enum addresses addresses_t;

    static std::vector<std::string> mAddressStrings;

    enum LogControl
    {
        Name = 0,
        Entry = 1,
        StopAll = 2,
    };
    typedef enum LogControl LogControl_t;

    static std::vector<std::string> mLogControl;

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

    static std::vector<std::string> mStatusControl;

    enum TransmissionControl
    {
        OK = 0,
        NOK = 1,
    };
    typedef enum TransmissionControl TransmissionControl_t;

    static std::vector<std::string> mTransmissionControl;


    enum DebugControl
    {
        GetRuntimeStats = 0,
        SetRuntimeStats = 1,
        GetErrorMessages = 2,

    };
    typedef enum DebugControl DebugControl_t;

    //This is used for enum to string conversion.
    static std::vector<std::string> mDebugControl;


	enum ParametersControl
	{
		SetTree = 0,
		GetTree = 1,
		Value = 2,
		Save = 3,
		Load = 4,
	};
	typedef enum ParametersControl ParametersControl_t;

	static std::vector<std::string> mParamControl;

    /**
     * Create an instance from a uint8_t array. Data is copied.
     * @param Payload Pointer to the array.
     * @param PayloadLength Length of the array.
     * @param offset.
     * @return Shared pointer to the created instance.
     */
    static ptr Create(const uint8_t* data, uint16_t length);

    /**
     *
     * @param create a QuadGSMsg.
     * @return Shared pointer to the created instance.
     */
    static ptr Create(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize);


    /**
     * Desructor
     */
    virtual ~QGS_MsgHeader();

    /**
     * @brief Get method for address field of the QuadGSMsg. The different
     * modules in the fc have different addresses.
     * @return the value of the address field.
     */
    uint8_t GetAddress() const;

    /**
     * @brief Set method for address field of the QuadGSMsg. The different
     * modules in the fc have different addresses.
     * @param address The value to be set in the address field.
     */
    void SetAddress(uint8_t address);

    /**
     * @brief Get method for control field of the QSP.This field
     * contains information to the receiving module.
     * @return control field of the QSP message.
     */
    uint8_t GetControl() const;

    /**
     * @brief Set method for control field of the QSP. This field
     * contains information to the receiving module.
     * @param control The control field of a QSP message.
     */
    void SetControl(uint8_t control);

    /**
     * @brief Get method for IsResend field of the QSP.This field
     * contains information to the receiving module.
     * @return IsResend field of the QSP message.
     */
    uint8_t GetIsResend() const;

    /**
     * @brief Set method for IsResend field of the QSP. This field
     * contains information to the receiving module.
     * @param IsResend The IsResend field of a QSP message.
     */
    void SetIsResend(uint8_t IsResend);

    /**
     * Get the complete package as a string.
     * @return A string containing the message.
     */
    virtual std::string toString() const;

    /**
     * Serialize to stream
     * @return
     */
    virtual BinaryOStream& stream(BinaryOStream& os) const;

    virtual BinaryIStream& stream(BinaryIStream& is);



protected:

	std::string ControlToString() const;


    /**
     * @brief Constructor
     * Creates a QuadSerialPacket.
     * @param data Pointer to data array containing the payload.
     * @param length Length of the data array.
     */
    QGS_MsgHeader(const uint8_t* data, uint16_t length);

    /**
     * Constructor taking the member variables respectively.
     * @param Address
     * @param Control
     * @param IsResend
     * @param PayloadSize
     */
    QGS_MsgHeader(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize);

    /**
     * @brief Set payload size
     * @param  size of the payload
     */
    void SetPayloadSize(uint16_t length);


    uint8_t mAddress;
    uint8_t mControl;
    uint8_t mIsResend;
    uint16_t mPayloadSize;
};


} /* namespace QuadGS */

#endif /* CORE_DATAMSG_SRC_QCMSGHEADER_H_ */
