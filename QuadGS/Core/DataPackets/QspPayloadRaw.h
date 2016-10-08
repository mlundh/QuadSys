/*
 * QspPayloadBase.h
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

#ifndef QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_
#define QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_
#include <memory>
#include <string.h>
#include <ostream>


namespace QuadGS {
/**
 * Data storage convenience class for QSP raw data.
 */
class QspPayloadRaw
{
public:
    /**
     * Shared pointer type for this type.
     */
    typedef std::shared_ptr<QspPayloadRaw> Ptr;

    /**
     * Deep copy of this instance.
     * @return A new instance, copy of this.
     */
    Ptr Copy();

   /**
    * Create an instance from a uint8_t array. Data is copied.
    * @param Payload Pointer to the array.
    * @param PayloadLength Length of the array.
    * @param offset.
    * @return Shared pointer to the created instance.
    */
   static Ptr Create(const uint8_t* Payload, uint16_t PayloadLength, uint16_t offset);


  /**
   * Create an instance from a uint8_t array. Data is copied.
   * @param Payload Pointer to the array.
   * @param PayloadLength Length of the array.
   * @param offset.
   * @return Shared pointer to the created instance.
   */
  static Ptr Create(const uint8_t* Payload, uint16_t PayloadLength);
  
    /**
     * Create an instance with length PayloadLength. The data
     * is allocated, but not assigned.
     * @param PayloadLength The length of the payload.
     * @return Shared pointer to the created instance.
     */
    static Ptr Create(uint16_t PayloadLength);

    /**
     * Get the array containing the payload.
     * @return pointer to the first element in the array.
     */
    uint8_t* getPayload() const;

    /**
     * Get the length of the payload array.
     * @return Length of the payload.
     */
    uint16_t getPayloadLength() const;

    /**
     * Set the payload length, used when the payload has been
     * modified.
     * @param size new size of the payload.
     */
    bool setPayloadLength(uint16_t size);

    /**
     * Get the payload represented as a string.
     * @return Payload represented as string.
     */
    std::string toString(int offset = 0) const;

    /**
     * Overloaded array index operators.
     */
    uint8_t& operator[](std::size_t idx);
    uint8_t operator[](std::size_t idx) const ;
    friend std::ostream& operator<< (std::ostream& stream, const QspPayloadRaw& pl);
    virtual ~QspPayloadRaw();
protected:  
    /**
     * Private constructors, use Create methods instead.
     */
    QspPayloadRaw( const uint8_t* Payload, uint16_t PayloadLength, uint16_t offset = 0);
    QspPayloadRaw(uint16_t PayloadLength);

    const static size_t mArrayLength;
    uint8_t* mPayload;
    uint16_t mPayloadLength;

private:
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_ */
