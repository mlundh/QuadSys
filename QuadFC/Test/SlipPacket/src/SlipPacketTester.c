/*
 * SlipPacketTester.c
 *
 * Copyright (C) 2020 Martin Lundh
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

#include "Test/SlipPacket/SlipPacketTester.h"
#include "Components/SLIP/inc/slip_packet.h"
#include <string.h>


uint8_t Slip_TestCreateDelete(TestFw_t *obj);
uint8_t Slip_TestParserCreateDelete(TestFw_t *obj);
uint8_t Slip_TestParserParse(TestFw_t *obj);
uint8_t Slip_TestParserParseChunks(TestFw_t *obj);


void Slip_GetTCs(TestFw_t *obj)
{
  TestFW_RegisterTest(obj, "Slip Create Delete", Slip_TestCreateDelete);
  TestFW_RegisterTest(obj, "SlipParser Create Delete", Slip_TestCreateDelete);
  TestFW_RegisterTest(obj, "SlipParser parse", Slip_TestParserParse);
  TestFW_RegisterTest(obj, "SlipParser parse chunks", Slip_TestParserParseChunks);

}

uint8_t Slip_TestCreateDelete(TestFw_t *obj)
{
  SLIP_t *packet = Slip_Create(255);
  Slip_Delete(packet);
  return 1;
}

uint8_t Slip_TestParserCreateDelete(TestFw_t *obj)
{
  SLIP_Parser_t *parser = SlipParser_Create(255);
  SlipParser_Delete(parser);
  return 1;
}


uint8_t Slip_TestParserParse(TestFw_t *obj)
{
  SLIP_Parser_t *parser = SlipParser_Create(255);
  uint8_t payload[] = "test~Stringxx";
  SLIP_t* packet = Slip_Create(255);

  uint32_t length = (sizeof(payload)/sizeof(payload[0]));
  Slip_Packetize(payload, length -2, length, packet);

  SLIP_t* packeParsed = Slip_Create(255);

  SLIP_Status_t result = SlipParser_Parse(parser, packet->payload, packet->packetSize, packeParsed);

  if(result != SLIP_StatusOK)
  {
    TEST_REPORT_ERROR(obj, "Slip Status not OK.");
    return 0;
  }
  int resultCmp = memcmp(payload, packeParsed->payload, packeParsed->packetSize);

  if(resultCmp != 0)
  {
    TEST_REPORT_ERROR(obj, "Did not get expected result from parsing.");
    return 0;
  }
  Slip_Delete(packet);
  Slip_Delete(packeParsed);
  SlipParser_Delete(parser);
  return 1;
}



uint8_t Slip_TestParserParseChunks(TestFw_t *obj)
{
  // create parser, test data and slip packet.
  SLIP_Parser_t *parser = SlipParser_Create(255);
  uint8_t payload[] = "test~Stringxx";
  SLIP_t* packet = Slip_Create(255);

  // packetize the data. 
  uint32_t length = (sizeof(payload)/sizeof(payload[0]));
  Slip_Packetize(payload, length -2, length, packet);

  // Create "receiver packet."
  SLIP_t* packeParsed = Slip_Create(255);

  // Parse the data. This funcion will be called mulitple times to 
  // verify the functionallity.
  SLIP_Status_t result = SlipParser_Parse(parser, packet->payload, packet->packetSize-4, packeParsed);

  if(result != SLIP_StatusCont)
  {
    TEST_REPORT_ERROR(obj, "Slip Status not OK.");
    return 0;
  }

  result = SlipParser_Parse(parser, &packet->payload[packet->packetSize-4], 4, packeParsed);

  if(result != SLIP_StatusOK)
  {
    TEST_REPORT_ERROR(obj, "Slip Status not OK.");
    return 0;
  }
  int resultCmp = memcmp(payload, packeParsed->payload, packeParsed->packetSize);

  if(resultCmp != 0)
  {
    TEST_REPORT_ERROR(obj, "Did not get expected result from parsing.");
    return 0;
  }
  Slip_Delete(packet);
  Slip_Delete(packeParsed);
  SlipParser_Delete(parser);
  return 1;
}