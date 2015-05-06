/*
 * ParameterPacket.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: martin
 */

#include "ParameterPacket.h"

namespace QuadGS {

ParameterPacket::ParameterPacket(uint8_t* data, uint8_t length):
        QuadSerialPacket(data, length)
{
    // TODO Auto-generated constructor stub

}

ParameterPacket::~ParameterPacket()
{
    // TODO Auto-generated destructor stub
}

} /* namespace QuadGS */
