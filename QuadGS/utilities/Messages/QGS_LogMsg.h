/*
 * QuadLogMsg.h
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#ifndef CORE_DATAMSG_QUADLOGMSG_H_
#define CORE_DATAMSG_QUADLOGMSG_H_

#include "QGS_Msg.h"
namespace QuadGS {

class QGS_LogMsg: public QuadGS::QGS_Msg {
public:
    typedef std::shared_ptr<QGS_LogMsg> ptr;

    /**
     * Create from a uint8_t array.
     * @param Data  pointer to the data.
     * @param Length length of the data.
     * @return A shared pointer to the message.
     */
    static ptr Create(const uint8_t* data, uint16_t length);

    /**
     * Create a message with the specified payload.
     * @return A shared pointer to the message.
     */
    static ptr Create(std::string payload);

    /**
     * Create an empty message.
     * @return A shared pointer to the message.
     */
    static ptr Create();

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
    BinaryIStream& stream(BinaryIStream& os);

    virtual ~QGS_LogMsg();

private:

    QGS_LogMsg();

    QGS_LogMsg(const std::string payload);

    QGS_LogMsg(const uint8_t* data, uint16_t length);

    std::string mPayload;

};
}/* namespace QuadGS */

#endif /* CORE_DATAMSG_QUADLOGMSG_H_ */
