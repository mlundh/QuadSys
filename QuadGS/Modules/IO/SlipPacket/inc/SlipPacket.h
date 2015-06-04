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
#include <stdexcept>

#include "QspPayloadRaw.h"
namespace QuadGS {

class SlipPacket
{
public:
  
  enum SlipControlOctets
  {
    frame_boundary_octet = 0x7E,
    frame_boundary_octet_replacement = 0x5E,
    control_escape_octet = 0x7D,
    control_escape_octet_replacement = 0x5D
  };
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
    static SlipPacketPtr Create(const uint8_t* data, uint8_t dataLength, bool isPayload);

    /**
     * @brief Create slip packet from data.
     * @param Payload The payload that should be packetized.
     * @return a pointer to the Slip packet.
     */
    static SlipPacketPtr Create(QspPayloadRaw::Ptr data, bool isPayload);

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
    SlipPacket(const uint8_t* data, uint8_t dataLength, bool isPayload);
    SlipPacket(QspPayloadRaw::Ptr data, bool isPayload);
    void initCrc();
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

    void addChecksumToPayload();
    void verifyChecksum();

    static bool mCrcInit;
    QspPayloadRaw::Ptr mPayload;
    QspPayloadRaw::Ptr mPacket;
    
    


};

} /* namespace QuadGS */

#endif /* _SLIPPACKET_H_ */
