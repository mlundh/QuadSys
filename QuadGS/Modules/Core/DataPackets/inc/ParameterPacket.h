/*
 * ParameterPacket.h
 *
 *  Created on: Apr 18, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_DATAPACKETS_PARAMETERPACKET_H_
#define QUADGS_MODULES_CORE_DATAPACKETS_PARAMETERPACKET_H_

#include "QuadSerialPacket.h"
#include <vector>

namespace QuadGS {



class ParameterPacket: public QuadSerialPacket
{
public:
    /**
     * @brief Constructor
     * Creates a ParameterPacket, assuming data and length
     * contains all needed info, including address and
     * control.
     * @param data Pointer to the data.
     * @length Length of the data array.
     */
    ParameterPacket(uint8_t* data, uint8_t length);

    /**
     * @brief Constructor.
     * Assumes the packet is a valid QSP.
     * @param packet A valid QSP packet.
     */
    ParameterPacket(const QspPayloadRaw::Ptr packet);

    /**
     * Desructor
     */
    virtual ~ParameterPacket();


};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_DATAPACKETS_PARAMETERPACKET_H_ */
