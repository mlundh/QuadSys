/*
 * QuadParamPacket.h
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

#ifndef QUADGS_MODULES_CORE_DATAPACKETS_INC_QUADPARAMPACKET_H_
#define QUADGS_MODULES_CORE_DATAPACKETS_INC_QUADPARAMPACKET_H_

#include "QuadSerialPacket.h"

namespace QuadGS {

class QuadParamPacket: public QuadSerialPacket
{
public:
    virtual ~QuadParamPacket();

    typedef std::shared_ptr<QuadParamPacket> Ptr;

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
      * @param packet Complete paramPacket contained in a raw packet.
      * @return Shared pointer to the created instance.
      */
     static Ptr Create(const QspPayloadRaw::Ptr packet);

     /**
      * Get field indicating if it is the last message in the
      * sequence.
      * @return 0 if not last, 1 otherwise.
      */
     uint8_t GetLastInSeq();

     /**
      * Set if message is the last in sequence.
      * @param isLast
      */
     void SetLastInSeq(uint8_t isLast);

     /**
      * Get the sequence number field in the param packet.
      * @return     Field value.
      */
    uint8_t GetSequenceNumber();

    /**
     * Set the sequence number of the package.
     * @param sequenceNumber    The sequence number of this packet.
     */
    void SetSequenceNumber(uint8_t sequenceNumber);

    /**
     * Get the payload length. Subtracts one from the message length
     * in the QSP due to the extra static field in the param message.
     * @return Payload length of the QSP packet.
     */
    uint16_t GetPayloadLength();

    /**
     * Get the payload of the message. This actuarially points into the
     * payload of the QSP by one byte. Convenience function only.
     * @return  Pointer to payload.
     */
    virtual QspPayloadRaw::Ptr GetPayload();

    /**
     * Print the payload as a formatted string.
     * @return  Human readable string.
     */
    virtual std::string PayloadToString();

protected:

    /**
     * Private constructors. Use create methods instead.
     */
    QuadParamPacket(const uint8_t* data, uint16_t length, uint8_t sequenceNr);

    QuadParamPacket(const uint8_t* data, uint16_t length);

    QuadParamPacket(const QspPayloadRaw::Ptr packet);


};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_DATAPACKETS_INC_QUADPARAMPACKET_H_ */
