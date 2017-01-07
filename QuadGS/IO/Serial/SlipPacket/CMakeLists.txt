list(APPEND src "SlipPacket.cpp")
list(APPEND dep "crc")

qgs_make_shared_lib( SlipPacket src dep)

list(APPEND src_DataMsgTest "test/SlipPacketTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "SlipPacket")

qgs_add_test(SlipPacketTest src_DataMsgTest dep_DataMsgTest)