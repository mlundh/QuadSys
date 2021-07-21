/*
* messagePoolTester.c
*
* Copyright (C) 2021  Martin Lundh
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
#include "string.h"
#include "Test/MessagePool/messagePoolTester.h"
#include "BaseComponents/MessagePool/inc/messagePool.h"
uint8_t MessagePoolTester_TestCreate(TestFw_t* Tobj);
uint8_t MessagePoolTester_TestAquireAll(TestFw_t* Tobj);
uint8_t MessagePoolTester_TestAquireReleaseAll(TestFw_t* Tobj);
uint8_t MessagePoolTester_TestAquireAllReleaseOne(TestFw_t* Tobj);
uint8_t MessagePoolTester_TestAquireAndFill(TestFw_t* Tobj);


void MessagePoolTester_GetTCs(TestFw_t* Tobj)
{
    TestFW_RegisterTest(Tobj, "MsgPool Test Create", MessagePoolTester_TestCreate);
    TestFW_RegisterTest(Tobj, "MsgPool Test Aquire ", MessagePoolTester_TestAquireAll);
    TestFW_RegisterTest(Tobj, "MsgPool Test Aquire Release ", MessagePoolTester_TestAquireReleaseAll);
    TestFW_RegisterTest(Tobj, "MsgPool Test Aquire all Release one", MessagePoolTester_TestAquireAllReleaseOne);
    TestFW_RegisterTest(Tobj, "MsgPool Test Aquire And Fill", MessagePoolTester_TestAquireAndFill);

}

uint8_t MessagePoolTester_TestCreate(TestFw_t* Tobj)
{
    messagePool_t* pool = messagePool_create(255, 6);
    if(!pool)
    {
        TEST_REPORT_ERROR(Tobj,"Could not create a messagePool_t." );
    }
    messagePool_delete(pool);
    return 1;
}

uint8_t MessagePoolTester_TestAquireAll(TestFw_t* Tobj)
{
    messagePool_t* pool = messagePool_create(255, 6);
    uint8_t* messages[8];
    for(int i = 0; i < 8; i++)
    {
        messages[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 6; i++)
    {
        if(!messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected. Iteration %d", i );
            return 0;
        }
    }
    for(int i = 6; i < 8; i++)
    {
        if(messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Got a memory area when none should be availible." );
            return 0;
        }
    }
    messagePool_delete(pool);
    return 1;
}

uint8_t MessagePoolTester_TestAquireReleaseAll(TestFw_t* Tobj)
{
    messagePool_t* pool = messagePool_create(255, 6);
    uint8_t* messages[8];
    for(int i = 0; i < 8; i++)
    {
        messages[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 6; i++)
    {
        if(!messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected. Iteration %d", i );
            return 0;
        }
    }
    for(int i = 6; i < 8; i++)
    {
        if(messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Got a memory area when none should be availible." );
            return 0;
        }
    }
    for(int i = 0; i < 4; i++)
    {
        messagePool_release(messages[i]);
    }
    uint8_t* messagesAfterRelease[8];
    for(int i = 0; i < 6; i++)
    {
        messagesAfterRelease[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 4; i++)
    {
        if(!messagesAfterRelease[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected after release. Iteration %d", i );
            return 0;
        }
    }
    
    for(int i = 4; i < 6; i++)
    {
        if(messagesAfterRelease[i])
        {
            TEST_REPORT_ERROR(Tobj,"Got a memory area when none should be availible after release." );
            return 0;
        }
    }
    messagePool_delete(pool);
    return 1;
}



uint8_t MessagePoolTester_TestAquireAllReleaseOne(TestFw_t* Tobj)
{
    messagePool_t* pool = messagePool_create(255, 6);
    uint8_t* messages[8];
    for(int i = 0; i < 8; i++)
    {
        messages[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 6; i++)
    {
        if(!messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected. Iteration %d", i );
            return 0;
        }
    }
    for(int i = 6; i < 8; i++)
    {
        if(messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Got a memory area when none should be availible." );
            return 0;
        }
    }

    messagePool_release(messages[3]);

    uint8_t* messageAfterRelease;

    messageAfterRelease = messagePool_aquire(pool, 255);

    if(!messageAfterRelease)
    {   
        TEST_REPORT_ERROR(Tobj,"Did not get a message even if there is a free messages in the pool." );
        return 0;
    }   
    messagePool_delete(pool);
    return 1;
}


uint8_t MessagePoolTester_TestAquireAndFill(TestFw_t* Tobj)
{
    messagePool_t* pool = messagePool_create(255, 6);
    uint8_t* messages[8];
    for(int i = 0; i < 6; i++)
    {
        messages[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 6; i++)
    {
        if(!messages[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected. Iteration %d", i );
            return 0;
        }
        memset(messages[i], 0, 255);
    }

    messages[7] = messagePool_aquire(pool, 200);
    
    if(messages[7])
    {
        TEST_REPORT_ERROR(Tobj,"Got a message when none should be availible.");
    }

    for(int i = 0; i < 6; i++)
    {
        messagePool_release(messages[i]);
    }
    uint8_t* messagesAfterRelease[8];
    for(int i = 0; i < 6; i++)
    {
        messagesAfterRelease[i] = messagePool_aquire(pool, 255);
    }
    for(int i = 0; i < 6; i++)
    {
        if(!messagesAfterRelease[i])
        {
            TEST_REPORT_ERROR(Tobj,"Did not get a memory area when expected after release. Iteration %d", i );
            return 0;
        }
    }
    
    messagePool_delete(pool);
    return 1;
}