/*
 * QuadLogMsg.h
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#ifndef CORE_DATAMSG_QUADLOGMSG_H_
#define CORE_DATAMSG_QUADLOGMSG_H_

#include "QGS_IoHeader.h"
namespace QuadGS {

class QGS_LogMsg: public QGS_IoHeader {
public:
    typedef std::unique_ptr<QGS_LogMsg> ptr;

    /**
     * Create from a uint8_t array.
     * @param Data  pointer to the data.
     * @param Length length of the data.
     */
    QGS_LogMsg(uint8_t control, const uint8_t* data, uint16_t length);

    /**
     * Create a message with the specified payload.
     */
    QGS_LogMsg(uint8_t control, const std::string payload);

    /**
     * Create an empty message.
     */
    QGS_LogMsg(const QGS_IoHeader& header);


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

    virtual ~QGS_LogMsg();

private:



    std::string mPayload;

};
}/* namespace QuadGS */

#endif /* CORE_DATAMSG_QUADLOGMSG_H_ */
