list(APPEND src "SlipPacket.cpp")
list(APPEND dep "crc")

qgs_make_shared_lib( SlipPacket src dep)

list(APPEND src_SlipTest "SlipPacketTest.cpp")
list(APPEND dep_SlipTest "gtest_main"
                         "SlipPacket"
                         "MsgBase"
                         "GS_FC_Param")

qgs_add_test(SlipPacketTest src_SlipTest dep_SlipTest)