/*
 * BinaryStream.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#include "../BinaryStream.h"

namespace QuadGS {


// TODO add check for next width!!
BinaryOStream& operator << (BinaryOStream& ostm, const char* val)
{
	int size = std::strlen(val);

	if(size<=0)
	{
		return ostm;
	}
	ostm.write(val, size);
	return ostm;
}

BinaryOStream& operator << (BinaryOStream& ostm, const std::string& val)
{
	ostm << val.c_str();
	return ostm;
}



// TODO add check for next width!!
BinaryIStream& operator >> (BinaryIStream& istm, std::string& val)
{
	char tmp;
	while (!istm.eof())
	{
		istm >> tmp;
		val.append(1,tmp);
	}
	return istm;
}


} /* namespace QuadGS */
