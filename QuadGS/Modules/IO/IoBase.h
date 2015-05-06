/*
 * IoBase.h
 *
 *  Created on: Apr 22, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#define QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#include "QspPayloadRaw.h"
#include <memory>
namespace QuadGS {

class IoBase
{
public:
    typedef std::shared_ptr<IoBase> ptr;

    IoBase();
    virtual ~IoBase();
    void RegisterDataCallback();
    bool write(QspPayloadRaw::Ptr ptr) = 0;

};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_IOBASE_H_ */
