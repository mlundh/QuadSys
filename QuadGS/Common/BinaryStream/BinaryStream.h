/*
 * BinaryStream.h
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#ifndef CORE_BINARYSTREAM_SRC_BINARYSTREAM_H_
#define CORE_BINARYSTREAM_SRC_BINARYSTREAM_H_
#include <vector>
#include <cstring>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace QuadGS {


/**
 * @class BinaryIStream
 *
 * Binary stream to pack data into a vector. Takes an arbitrary number of bits
 * from the input as determined by either the SetWidth() function, or the width
 * of the data.
 *
 * Note that negative numbers are only presented correctly when the whole data
 * width of the input data type is used.
 */
class BinaryIStream {
friend BinaryIStream& operator >> (BinaryIStream& istm, char* val);

public:
    virtual ~BinaryIStream(){}
    BinaryIStream()
    : mIndex(0)
    , mBitIdx(7)
    , mNextWidth(-1)
    , mNextCharLength(0)
    {

    }

    /**
     * Create a stream from a uint8_t pointer and a size. Will copy the data.
     * @param mem	Memory to read from.
     * @param size	Sixe of the data.
     */
    BinaryIStream(const uint8_t* mem, size_t size)
    : mIndex(0)
    , mBitIdx(7)
    , mNextWidth(-1)
    , mNextCharLength(0)
    {
        open(mem, size);
    }

    /**
     * Create from a vector. Will copy the data.
     * @param vec The vectorn containing the data.
     */
    BinaryIStream(const std::vector<unsigned char>& vec)
    : mIndex(0)
    , mBitIdx(7)
    , mNextWidth(-1)
    , mNextCharLength(0)
    {
        mVec.reserve(vec.size());
        mVec.assign(vec.begin(), vec.end());
    }

    /**
     * Get the internal vector. TNote that the vector is returned by reference,
     * and hence the BinaryIStream object still owns the vector, and destroys it
     * if the stream gets destroyed.
     * @return Reference to the internal vector.
     */
    const std::vector<unsigned char>& getInternalVec()
	        {
        return mVec;
	        }

    /**
     * Open the stream. This is for internal use.
     * @param mem	Data to write into the stream.
     * @param size	Size of the data to write.
     */
    void open(const uint8_t * mem, size_t size)
    {
        mIndex = 0;
        mVec.clear();
        mVec.reserve(size);
        mVec.assign(mem, mem + size);
    }

    /**
     * Close the stream. Clears the vector.
     */
    void close()
    {
        mVec.clear();
    }

    /**
     * Check if there is data left in the stream.
     * @return true if eof, false otherwise.
     */
    bool eof() const
    {
        return mIndex >= mVec.size();
    }

    /**
     * Read from the stream. This is for internal use only.
     * @param data Data read from the stream.
     */
    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void read(T& data)
    {
        // Make sure there is enough data in the stream.
        {
            if(eof())
            {
                throw std::runtime_error("Premature end of array!");
            }

            unsigned int sizeToWrite = 0;
            if(!mNextWidth)
            {
                sizeToWrite = sizeof(T);
            }
            else
            {
                sizeToWrite = mNextWidth/8;
            }
            if(sizeToWrite > mVec.size())
            {
                throw std::runtime_error("Premature end of array!");
            }
        }
        // If we are aligned and reading a multiple of eight, then read!
        if(mBitIdx == 7 && (mNextWidth < 0 || (mNextWidth % 8 == 0)))
        {
            int width;
            if(mNextWidth > 0)
            {
                width = mNextWidth / 8;
                width -= 1;
            }
            else
            {
                width = sizeof(data) - 1;
            }

            data = 0;
            uint8_t tmpVal = 0;
            for( int i = width; i >= 0; i-- )
            {
                tmpVal = mVec[mIndex++];
                data |= (T)tmpVal << (8*i);
            }
        }
        else if((mBitIdx <= 7)) // sanity check. mBitIdx should always be between 0 and 7.
        {
            if(mNextWidth < 0)
            {
                mNextWidth = sizeof(data)*8;
            }
            int bitsInCurrentByte = mBitIdx + 1;

            // If bits are entirely located in the current byte - copy and update.
            if(bitsInCurrentByte >= mNextWidth)
            {

                // Get bits and populate data.
                uint8_t bitmask = ((1 << mNextWidth) - 1);
                uint8_t tmpData = mVec[mIndex];
                int shift = (mBitIdx - mNextWidth + 1);
                tmpData >>= shift;
                tmpData &= bitmask;
                data = tmpData;

                // Update mBitIdx and mIndex.
                mBitIdx -= mNextWidth;
                if(mBitIdx < 0)
                {
                    mBitIdx = 8 + mBitIdx;
                    mIndex++;
                }


            }
            else
            {
                // First, get the number of bits in the last byte in the vector.
                int bitsInLastByte = (mNextWidth - bitsInCurrentByte) % 8;

                // Find the number of bytes used by the width.

                int wholeBytes = 0;
                {
                    int BitsUsedRoundUp = 0;
                    int remainder = mNextWidth % 8;
                    if (remainder == 0)
                    {
                        BitsUsedRoundUp = mNextWidth;
                    }
                    else
                    {
                        BitsUsedRoundUp = mNextWidth + 8 - remainder;
                    }
                    wholeBytes = (BitsUsedRoundUp - (bitsInCurrentByte + bitsInLastByte)) / 8;
                }

                // Get top bits and write them in the right position to data.
                uint8_t bitmask_first_bits =  ((1 << bitsInCurrentByte) - 1);
                uint8_t topBits = mVec[mIndex++];
                topBits &= bitmask_first_bits;
                int shiftTop = (bitsInLastByte + wholeBytes*8);
                data = (T)topBits << shiftTop;


                // Write the whole bytes in the middle.
                for (int i = wholeBytes; i > 0; i --)
                {
                    int shiftFactor = (8*(i-1) + bitsInLastByte);
                    T shiftedData = ((T)(mVec[mIndex++]) << shiftFactor);
                    data |= shiftedData;
                }
                // Write last bits to output if there is any data to write.
                if(bitsInLastByte > 0)
                {

                    // Last bits to be written.
                    uint8_t bitmaskLastBits = ((1 << bitsInLastByte) - 1);
                    uint8_t lastBits = ((uint8_t)mVec[mIndex]);
                    uint8_t shiftLastBits = 8 - bitsInLastByte;
                    lastBits >>= shiftLastBits;

                    lastBits &= bitmaskLastBits;
                    data |= lastBits;
                }
                // Update mBitIdx
                mBitIdx -= (mNextWidth%8);
                if(mBitIdx < 0)
                {
                    mBitIdx = 8 + mBitIdx;
                }
            }
        }
        mNextWidth = -1;
    }

    void read(char* p, size_t size)
    {

        for(size_t i = 0; i < size; i++)
        {
            read(p[i]);
        }
    }
    void read(std::vector<unsigned char>& vec)
    {
        for(size_t i = 0; i < vec.size(); i++)
        {
            read(vec[i]);
        }

    }
    void setNextBits(int i)
    {
        mNextWidth = i;
    }

    void setNextCharLength(int i)
    {
    	mNextCharLength = i;
    }

private:

    std::vector<unsigned char> mVec;
    size_t mIndex;
    int mBitIdx;
    int mNextWidth;
    int mNextCharLength;
};


/**
 * Enable streaming of integral types for the BinaryIStream.
 * @param istm	Binary stream object.
 * @param val	Value to stream.
 * @return		binary stream object.
 */
template<typename T,
typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
BinaryIStream& operator >> (BinaryIStream& istm, T& val)
{
    istm.read(val);

    return istm;
}


/**
 * Enable streaming of a c-style string. Only use with c-style, null terminated strings.
 * Ignores next width parameter(only used for the first byte). Use SetByte for setting length of the string.
 * @param istm
 * @param val
 * @return
 */
BinaryIStream& operator >> (BinaryIStream& istm, char* val);

/**
 * Stream a string. Only uses next width for the first byte.
 * @param istm
 * @param val
 * @return
 */
BinaryIStream& operator >> (BinaryIStream& istm, std::string& val);

/**
 * @class BinaryOStream
 *
 * Binary output stream for binary data. Will take an arbitrary number of bits
 * from the input data and pack them into the output array. The width is
 * streamed into the stream by the function SetWidth().
 *
 * Please note that negative numbers are only represented correctly if the whole
 * data width of the input type is written.
 */

class BinaryOStream
{
public:
friend	BinaryOStream& operator << (BinaryOStream& ostm, const char* val);

    BinaryOStream()
:mVec()
,mBitIdx(7)
,mNextWidth(-1)
,mNextCharLength(0)
{

}

    void close()
    {
        mVec.clear();
    }

    const std::vector<unsigned char>& get_internal_vec()
            {
        return mVec;
            }

    /**
     * Write function for integer types only.
     * If the stream is byte alligned, and we are trying to write
     * an integer number of bytes, then it just writes them.
     *
     * If trying to write less than the number of bits left in the
     * current byte, then write into the current byte.
     *
     * If trying to write more than the nuber of bits left in the
     * current byte, then extract the first and last bits according to:
     *
     *
     * current index:      5
     * data:     		 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
     * write to: | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
     *
     * Extract the last bits, here [ 1, 0 ], then shift down as many
     * positions as the number of last bits, here 2. This leaves everything
     * but the extracted bits.
     *
     * Then calculate the number of bits in the top byte, here 6 bytes. Extract and
     * mask from data. This leaves a number of whole bytes in data.
     *
     * Write everything into the underlying array.
     *
     */
    template <typename T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void write(const T& t)
    {
        T data = t;
        // If we are aligned and writing a multiple of eight, then write!
        if(mBitIdx == 7 && (mNextWidth < 0 || (mNextWidth % 8 == 0)))
        {
            int width;
            if(mNextWidth > 0)
            {
                width = mNextWidth / 8;
                width -= 1;
            }
            else
            {
                width = sizeof(data) - 1;
            }

            for(int i = width; i >= 0; i--)
            {
                mVec.push_back((uint8_t)(data >> (8*i)));
            }
        }
        else if((mBitIdx <= 7)) // sanity check. mBitIdx should always be between 0 and 7.
        {
            if(mNextWidth < 0)
            {
                mNextWidth = sizeof(data)*8;
            }
            int bitsLeftInFirstByte = mBitIdx + 1;

            // If bits fit in the first byte - write and update.
            if(bitsLeftInFirstByte >= mNextWidth)
            {
                // If it is the first bits to be written we have to create a new byte,
                // the same applies if we are at bit index 7.
                if(mVec.empty() || mBitIdx == 7)
                {
                    mVec.push_back(0);
                }

                // Get last bits and save them for later. Shift them out of the data.
                uint8_t bitmask_last_bits = ((1 << mNextWidth) - 1);
                uint8_t last_bits = ((uint8_t)data & bitmask_last_bits);
                uint8_t result = (last_bits << (mBitIdx - mNextWidth + 1));
                mVec.back() |= result;

                // Update mBitIdx
                mBitIdx -= mNextWidth;
                if(mBitIdx < 0)
                {
                    mBitIdx = 8 + mBitIdx;
                }
            }
            else
            {
                // First, extract and save the last bits.Then updata data to contain the rest.
                int bitsInLastByte = (mNextWidth - bitsLeftInFirstByte) % 8;

                // Get last bits and save them for later. Shift them out of the data.
                uint8_t bitmask_last_bits = ((1 << bitsInLastByte) - 1);
                uint8_t last_bits = ((uint8_t)data & bitmask_last_bits);
                data = (T)data >> bitsInLastByte;

                // Get the first bits. Then remove the top bits from the data.
                // Data is shifted so that the valid data is in the lower
                // bits of the top byte.
                uint8_t bitmask_first_bits =  ((1 << bitsLeftInFirstByte) - 1);

                // round up to nearest multiple of 8 for shifting.
                int TopBitPos = 0; // Top bit in top byte used by mNextWidth.
                int remainder = mNextWidth % 8;
                if (remainder == 0)
                {
                    TopBitPos = mNextWidth;
                }
                else
                {
                    TopBitPos = mNextWidth + 8 - remainder;
                }

                // get top byte that we are interested in.
                uint8_t first_bits = (uint8_t)(data >> (TopBitPos - 8));

                first_bits &= bitmask_first_bits; // mask away the bits we are not interested in.
                T bitmaskData = (( (T)1 << (mNextWidth - bitsLeftInFirstByte -  bitsInLastByte)) - 1 );
                data = (T)data & bitmaskData;

                // Clear bits in the last element of the vector needed for the first bits of the new data.
                if(mVec.empty())
                {
                    mVec.push_back(0);
                }
                mVec.back() &= ~bitmask_first_bits;
                mVec.back() |= first_bits;

                for (int i = TopBitPos/8 - 2; i >= 0; i --)
                {
                    mVec.push_back((uint8_t)(data >> (8*i)));
                }

                // Write last bits to the vector if there is any data to write.
                if(bitsInLastByte > 0)
                {
                    mVec.push_back((uint8_t)(last_bits << (mBitIdx + 1)));
                }
                // Update mBitIdx
                mBitIdx -= (mNextWidth%8);
                if(mBitIdx < 0)
                {
                    mBitIdx = 8 + mBitIdx;
                }
            }
        }
        mNextWidth = -1;
    }
    void write(const char* p, size_t size)
    {
        for(size_t i=0; i<size; ++i)
        {
            write(p[i]);
        }
    }
    void setNextBits(int i)
    {
        mNextWidth = i;
    }

    void setNextCharLength(int i)
    {
    	mNextCharLength = i;
    }
    //private:
    std::vector<uint8_t> mVec;
    int mBitIdx;
    int mNextWidth;
    int mNextCharLength;
};

/**
 * Enable integer streaming to the output stream.
 * @param ostm	Output stream.
 * @param val	Value to stream.
 * @return		Output stream.
 */
template<typename T,
typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
BinaryOStream& operator << (BinaryOStream& ostm, const T& val)
{
    ostm.write(val);

    return ostm;
}
/**
 * Enable streaming of a c-style string. Only use with c-style, null terminated strings.
 * Ignores next width parameter(only used for the first byte). Use SetByte for setting length of the string.
 * @param ostm
 * @param val
 * @return
 */
BinaryOStream& operator << (BinaryOStream& ostm, const char* val);

/**
 * Stream a string. Streams the whole string. Does not use the next width exept for the
 * first byte.
 * @param ostm
 * @param val
 * @return
 */
BinaryOStream& operator << (BinaryOStream& ostm, const std::string& val);

/**
 * @class SetNxtBits
 * Utility class for setting number of bits in the binary streams.
 */
struct SetNxtBits
{
    int mBits;
};

/**
 * Functin for setting number of bits in the stream.
 * @param nBits	number of bits to set.
 * @return
 */
inline SetNxtBits
SetBits(int nBits)
{
    return { nBits };
}


/**
 * @class SetNxtBytes
 * Utility class for setting number of bits in the binary streams.
 */
struct SetNxtBytes
{
    int mBytes;
};

/**
 * Functin for setting number of bits in the stream.
 * @param nBits	number of bits to set.
 * @return
 */
inline SetNxtBytes
SetBytes(int nBytes)
{
    return { nBytes };
}

/**
 * Stream operator for setting next bit width of a binary input stream.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, SetNxtBits bits)
{
    stream.setNextBits(bits.mBits);
    return stream;
}

/**
 * Stream operator for setting next bit width of a binary output stream.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, SetNxtBits bits)
{
    stream.setNextBits(bits.mBits);
    return stream;
}



/**
 * Stream operator for setting next bit width of a binary input stream.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, SetNxtBytes bytes)
{
    stream.setNextCharLength(bytes.mBytes);
    return stream;
}

/**
 * Stream operator for setting next bit width of a binary output stream.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, SetNxtBytes bytes)
{
    stream.setNextCharLength(bytes.mBytes);
    return stream;
}
} /* namespace QuadGS */

#endif /* CORE_BINARYSTREAM_SRC_BINARYSTREAM_H_ */
