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
/**
 *
 *
 * Big endian, MSB0
 *
 * A parameter frame extends the QSP by adding a new
 * static field, the first byte in the payload:
 *
 *  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 *  +-------------------------------+
 * 0|            address            |
 *  +-------------------------------+
 * 1| R|         Control            |
 *  +-------------------------------+
 * 2|          payload size H       |
 *  +-------------------------------+
 * 3|          payload size L       |
 *  +-------------------------------+
 * 3| L|       sequence number      |
 *  +-------------------------------+
 * 4|           payload             |
 *  +-------------------------------+
 *
 *  The R field indicates resend.
 *  The L field indicates if it is the last in a sequence.
 */

#include "QGS_IoHeader.h"

namespace QuadGS {

class QGSParamMsg: public QGS_IoHeader
{
public:

	//QGSParamMsg(const uint8_t* data, uint16_t length, uint8_t sequenceNr, uint8_t lastInSequence);

	/**
	 * Create an instance from a uint8_t array. Data is copied.
	 * @param Payload Pointer to the array.
	 * @param PayloadLength Length of the array.
	 */
	QGSParamMsg(const QGS_IoHeader& header, const uint8_t* data, uint16_t length);

	//TODO make control data length ctor.

	QGSParamMsg(const QGS_IoHeader& header);

	/**
	 * Create from payload, sequence number and lastInSequence.
	 * @param Control		The type of parameter message.
	 * @param payload 		The string describing the parameter payload.
	 * @param sequenceNr	Sequence number of the payload message.
	 * @param lastInSequence  If the message is the last in the sequence.
	 * @return
	 */
	QGSParamMsg(uint8_t Control, const std::string payload, uint8_t sequenceNr, uint8_t lastInSequence);


	virtual ~QGSParamMsg();

	typedef std::shared_ptr<QGSParamMsg> ptr;

	/**
	 * Get field indicating if it is the last message in the
	 * sequence.
	 * @return 0 if not last, 1 otherwise.
	 */
	uint8_t GetLastInSeq() const;

	/**
	 * Set if message is the last in sequence.
	 * @param isLast
	 */
	void SetLastInSeq(uint8_t isLast);

	/**
	 * Get the sequence number field in the param packet.
	 * @return     Field value.
	 */
	uint8_t GetSequenceNumber() const;

	/**
	 * Set the sequence number of the package.
	 * @param sequenceNumber    The sequence number of this packet.
	 */
	void SetSequenceNumber(uint8_t sequenceNumber);

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
	 * Print the payload as a formatted string.
	 * @return  Human readable string.
	 */
	virtual std::string toString() const;

	/**
	 * Stream the class into a binaryOstream. This will serialize the whole class, including the header.
	 */
	BinaryOStream& stream(BinaryOStream& os) const;

	/**
	 * Stream into the class instance. This will not populate the fields created by the header since
	 * this information is required to create a package of this type.
	 */
	BinaryIStream& stream(BinaryIStream& is);

protected:


	uint8_t mSequenceNumber;
	uint8_t mLastInSequence;
	std::string mPayload;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_DATAPACKETS_INC_QUADPARAMPACKET_H_ */
