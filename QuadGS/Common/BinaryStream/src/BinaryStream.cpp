/*
 * BinaryStream.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#include "BinaryStream.h"

namespace QuadGS {



BinaryOStream& operator << (BinaryOStream& ostm, const char* val)
{
	int size = 0;
	if(ostm.mNextCharLength > 0)
	{
		size = ostm.mNextCharLength;
	}
	else
	{
		size = std::strlen(val);
	}
	if(size<=0)
	{
		return ostm;
	}
	ostm.write(val, size);
	ostm.mNextCharLength = 0;

	return ostm;
}

BinaryIStream& operator >> (BinaryIStream& istm, char* val)
{
	int size = 0;
	if(istm.mNextCharLength > 0)
	{
		size = istm.mNextCharLength;
	}
	if(size<=0)
	{
		return istm;
	}
	istm.read(val, size);
	istm.mNextCharLength = 0;

	return istm;
}


BinaryOStream& operator << (BinaryOStream& ostm, const std::string& val)
{
	int length = val.length();
	ostm << length;
	ostm << val.c_str();
	return ostm;
}



BinaryIStream& operator >> (BinaryIStream& istm, std::string& val)
{
	istm.setNextBits(0);
	int nxtWidth = 0;
	istm >> nxtWidth;
	char tmp = '\0';
	while (!istm.eof() && (nxtWidth > 0))
	{
		istm >> tmp;
		val.append(1,tmp);
		nxtWidth--;
	}
	return istm;
}


} /* namespace QuadGS */
