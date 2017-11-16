/*
 * QuadLogMsg.h
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#ifndef CORE_DATAMSG_QUADLOGMSG_H_
#define CORE_DATAMSG_QUADLOGMSG_H_

#include "QuadGSMsg.h"
namespace QuadGS {

class QuadLogMsg: public QuadGS::QuadGSMsg {
public:
    typedef std::shared_ptr<QuadLogMsg> ptr;
    friend BinaryOStream& operator<< (BinaryOStream& os, const QuadLogMsg& pl);
    friend BinaryIStream& operator>> (BinaryIStream& is, QuadLogMsg& pl);


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

    virtual ~QuadLogMsg();

private:

    QuadLogMsg();

    QuadLogMsg(const std::string payload);

    QuadLogMsg(const uint8_t* data, uint16_t length);

    std::string mPayload;

};
}/* namespace QuadGS */

#endif /* CORE_DATAMSG_QUADLOGMSG_H_ */
