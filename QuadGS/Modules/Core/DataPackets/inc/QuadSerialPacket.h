/*
 * QuadSerialPacket.h
 *
 *  Created on: Feb 14, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_
#define QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_


#include "QspPayloadRaw.h"

namespace QuadGS {

class QuadSerialPacket
{
public:
    typedef std::shared_ptr<QuadSerialPacket> Ptr;

    enum addresses
    {
        Parameters = 1,
        Log = 2,
        FunctionCall = 3,
        Control = 4, // TODO remove from QSP, should reside a layer beneath.
    };
    enum ParametersControl
    {
        SetTree = 2,
        GetTree = 3,
        Value = 4,
    };
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
     * Get the payload length.
     * @return Payload length of the QSP packet.
     */
    uint16_t GetPayloadLength();

    /**
     * Get the payload contained in the QSP, this is the QSP without header.
     * @return Pointer to the payload.
     */
    QspPayloadRaw::Ptr GetPayload();

    /**
     * Get the whole message as raw data.
     * @return Pointer to raw data.
     */
    QspPayloadRaw::Ptr GetRawData();

protected:

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
    const static uint8_t mHeaderSize;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_ */
