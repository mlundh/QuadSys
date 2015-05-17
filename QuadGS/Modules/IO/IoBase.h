/*
 * IoBase.h
 *
 *  Created on: Apr 22, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#define QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#include "QspPayloadRaw.h"
#include "Core.h"
#include <memory>
namespace QuadGS {

class IoBase
{
public:
    IoBase(){}
    virtual ~IoBase(){}
    virtual void write( QspPayloadRaw::Ptr ptr) = 0;
    virtual void startRead( void ) = 0;
    virtual bool set_read_callback( ) = 0;
    virtual std::vector< Command::ptr > getCommands( ) = 0;
    virtual std::string getStatus( ) = 0;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_IOBASE_H_ */
