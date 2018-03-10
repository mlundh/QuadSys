/*
 * QuadDebugMsg.h
 *
 *  Created on: Jan 7, 2017
 *      Author: martin
 */

#ifndef CORE_DATAMSG_QUADDEBUGMSG_H_
#define CORE_DATAMSG_QUADDEBUGMSG_H_
#include <string>

#include "QGS_IoHeader.h"

namespace QuadGS {

class QGS_DebugMsg : public QGS_IoHeader
{
public:
    typedef std::shared_ptr<QGS_DebugMsg> ptr;

    /**
     * Create an empty message.
     */
    QGS_DebugMsg(const QGS_IoHeader& header);

    /**
     * Create a message with the specified payload.
     */
    QGS_DebugMsg(uint8_t Control, const std::string payload);

    /**
     * Create from a uint8_t array.
     * @param Data	pointer to the data.
     * @param Length length of the data.
     */
    QGS_DebugMsg(uint8_t control, const uint8_t* data, uint16_t length);

    /**
     * Get the payload.
     * @return     payload.
     */
    std::string GetPayload() const;

    /**
     * Set the payload
     * @param payload    The payload.
     */
    void Setpayload(std::string payload);

    /**
     * Prints the message in a human readable form.
     * @return
     */
    virtual std::string toString() const;

    /**
     * Implement the interface, this is a utility to stream the class.
     */
    BinaryOStream& stream(BinaryOStream& os) const;

    /**
     * Implement the interface, this is a utility to stream the class.
     */
    BinaryIStream& stream(BinaryIStream& is);

    virtual ~QGS_DebugMsg();

private:


    std::string mPayload;
};

} /* namespace QuadGS */

#endif /* CORE_DATAMSG_QUADDEBUGMSG_H_ */
