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
    typedef std::shared_ptr<QuadSerialPacket> QspPtr;

    /**
     * @brief Constructor
     * Creates a QuadSerialPacket, assuming data and length
     * contains all needed info.
     */
    QuadSerialPacket(uint8_t* data, uint8_t length);

    /**
     * @brief Constructor.
     * Assumes the packet is a valid QSP.
     */
    QuadSerialPacket(const QspPayloadRaw::Ptr packet);

    /**
     * Desructor
     */
    virtual ~QuadSerialPacket();

    /**
     * @brief Get method for address field of the QSP.
     */
    uint8_t GetAddress();

    /**
     * @brief Set method for address field of the QSP.
     */
    void SetAddress(uint8_t address);

    /**
     * @brief Get method for control field of the QSP.
     * @return control field of the QSP message.
     */
    uint8_t GetControl();

    /**
     * @brief Set method for control field of the QSP.
     * @param control The control field of a QSP message.
     */
    void SetControl(uint8_t control);

    /**
     * @brief Replace the payload of the QSP message.
     * @param pl A pointer to the new payload.
     * @return true if successful, false otherwise.
     */
    bool SetPayload(QspPayloadRaw::Ptr pl);

    /**
     * Get the payload.
     * @return Pointer to the payload.
     */
    QspPayloadRaw::Ptr GetPayload();

protected:

    /**
     * @brief Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt8(int8_t value, uint start);


    /**
     * @brief Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt16(int16_t value, uint start);

    /**
     * Serialize an int16, value, and put it in the buffer.
     * @param value Integer to serialize.
     * @param start Start position in mPayload.
     * @return true is successful, false otherwise.
     */
    bool SerializeInt32(int32_t value, uint start);

    /**
     * @brief Deserialize an int8 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     */
    uint8_t DeserializeInt8(uint start);

    /**
     * @brief Deserialize an int16 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     */
    uint16_t DeserializeInt16(uint start);

    /**
     * @brief Deserialize an int32 located at byte start in mPayload.
     * @param start the index of mPayload where the serialized
     *        value is located.
     * @return The deserialized value.
     *      */
    uint32_t DeserializeInt32(uint start);



    QspPayloadRaw::Ptr mPayload;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QUADSERIALPACKET_H_ */
