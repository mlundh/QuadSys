/*
 * QuadSerialPacket.h
 *
 * Copyright (C) 2015 Martin Lundh
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

/**
 * @file QuadSerialPacket.h
 *
 * A QSP is defined as:
 *
 *
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 *  +-------------------------------+
 * 1|            address            |
 *  +-------------------------------+
 * 2| R|         Control            |
 *  +-------------------------------+
 * 3|          payload size         |
 *  +-------------------------------+
 * 4|           payload             |
 *  +-------------------------------+
 *
 *  Where the R field indicates resend.
 *
 *
 *
 */

#ifndef QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_
#define QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_


#include "QspPayloadRaw.h"
#include <vector>
namespace QuadGS {


class QuadSerialPacket
{
public:
    typedef std::shared_ptr<QuadSerialPacket> Ptr;

    enum addresses
    {
        Parameters = 0,
        Log = 1,
        FunctionCall = 2,
        Status = 3, // TODO remove from QSP, should reside a layer beneath.
    };
    typedef enum addresses addresses_t;

    static std::vector<std::string> mAddressStrings;

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



    /**
     * Create an instance from a uint8_t array. Data is copied.
     * @param Payload Pointer to the array.
     * @param PayloadLength Length of the array.
     * @param offset.
     * @return Shared pointer to the created instance.
     */
    static Ptr Create(const uint8_t* data, uint16_t length);

    /**
     *
     * @param packet Complete QSP contained in a raw packet.
     * @return Shared pointer to the created instance.
     */
    static Ptr Create(const QspPayloadRaw::Ptr packet);


    /**
     * Desructor
     */
    virtual ~QuadSerialPacket();

    /**
     * @brief Get method for address field of the QSP. The different
     * modules in the fc have different addresses.
     * @return the value of the address field.
     */
    uint8_t GetAddress();

    /**
     * @brief Set method for address field of the QSP. The different
     * modules in the fc have different addresses.
     * @param address The value to be set in the address field.
     */
    void SetAddress(uint8_t address);

    /**
     * @brief Get method for control field of the QSP.This field
     * contains information to the receiving module.
     * @return control field of the QSP message.
     */
    uint8_t GetControl();

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
    uint8_t GetIsResend();

    /**
     * @brief Set method for IsResend field of the QSP. This field
     * contains information to the receiving module.
     * @param IsResend The IsResend field of a QSP message.
     */
    void SetIsResend(uint8_t IsResend);

    /**
     * Get the payload length.
     * @return Payload length of the QSP packet.
     */
    uint16_t GetPayloadLength();

    /**
     * Get the payload contained in the QSP, this is the QSP without header.
     * @return Pointer to the payload.
     */
    virtual QspPayloadRaw::Ptr GetPayload();

    /**
     * Get the whole message as raw data.
     * @return Pointer to raw data.
     */
    QspPayloadRaw::Ptr GetRawData();

    /**
     * Get the complete package as a string.
     * @return A string containing the message.
     */
    std::string ToString();
protected:

    /**
     * Get the header of the message as a string.
     * @return A string containing the header.
     */
    virtual std::string HeaderToString();

    /**
     * Get the payload as a string.
     * @return  The payload as a human readable string.
     */
    virtual std::string PayloadToString();

    QuadSerialPacket(const uint8_t* data, uint16_t data_length, uint8_t offset);

    /**
     * @brief Constructor
     * Creates a QuadSerialPacket.
     * @param data Pointer to data array containing the payload.
     * @param length Length of the data array.
     */
    QuadSerialPacket(const uint8_t* data, uint16_t length);

    /**
     * @brief Constructor.
     * Assumes the packet is a valid QSP.
     */
    QuadSerialPacket(const QspPayloadRaw::Ptr packet);


    /**
     * @brief Set payload length
     * @param  Length of the payload
     */
    void SetPayloadLength(uint16_t length);

    /**
     * @brief Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt8(int8_t value, uint32_t start);


    /**
     * @brief Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt16(int16_t value, uint32_t start);

    /**
     * Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt32(int32_t value, uint32_t start);

    /**
     * @brief Deserialize an int8 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     */
    int8_t DeserializeInt8(uint32_t start);

    /**
     * @brief Deserialize an int16 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     */
    int16_t DeserializeInt16(uint32_t start);

    /**
     * @brief Deserialize an int32 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     *      */
    int32_t DeserializeInt32(uint32_t start);


    QspPayloadRaw::Ptr mPayload;

    const static uint8_t mHeaderSize{4};
};



} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_ */
