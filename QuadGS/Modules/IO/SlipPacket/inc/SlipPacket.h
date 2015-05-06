/*
 * SlipPacket.h
 *
 *  Created on: Feb 21, 2015
 *      Author: martin
 */

#ifndef _SLIPPACKET_H_
#define _SLIPPACKET_H_
#include <memory>
#include <string.h>

#include "QspPayloadRaw.h"
namespace QuadGS {

class SlipPacket
{
public:
    /**
     * typedefed handle to a slip packet.
     */
    typedef std::shared_ptr<SlipPacket> SlipPacketPtr;

    /**
     * @brief Create and decode a slip packet from a raw message.
     * @param Payload The raw slip packet.
     * @param PayloadLength The length of the packet.
     * @return A pointer to the Slip packet.
     */
    static SlipPacketPtr Create(uint8_t* Payload, uint8_t PayloadLength);

    /**
     * @brief Create and encode a slip packet from data.
     * @param Payload The payload that should be packetized.
     * @return a pointer to the Slip packet.
     */
    static SlipPacketPtr Create(QspPayloadRaw::Ptr Payload);

    /**
     * @brief Get the Payload carried in the packet.
     * @return A pointer to the decoded payload.
     */
    QspPayloadRaw::Ptr GetPayload();

    /**
     * @brief Get the slip packet that contains the payload.
     * @return A pointer to the encoded packet.
     */
    QspPayloadRaw::Ptr GetPacket();

    /**
     * @brief destructor.
     */
    virtual ~SlipPacket();

private:
    SlipPacket(uint8_t* Payload, uint8_t PayloadLength);
    SlipPacket(QspPayloadRaw::Ptr Payload);

    /**
     * @brief Encode the payload.
     * @return True if success.
     */
    bool Encode();

    /**
     * @brief Decode the packet.
     * @return True if success.
     */
    bool Decode();

    QspPayloadRaw::Ptr mPayload;
    QspPayloadRaw::Ptr mPacket;

    enum SlipControlOctets
    {
      frame_boundary_octet = 0x7E,
      frame_boundary_octet_replacement = 0x5E,
      control_escape_octet = 0x7D,
      control_escape_octet_replacement = 0x5D
    };
};

} /* namespace QuadGS */

#endif /* _SLIPPACKET_H_ */
