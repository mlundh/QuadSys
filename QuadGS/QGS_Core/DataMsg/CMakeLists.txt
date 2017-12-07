list(APPEND src "QCMsgHeader.cpp"
                "QuadGSMsg.cpp"
                "QuadParamPacket.cpp"
                "QuadDebugMsg.cpp"
                "QuadLogMsg.cpp")
                
list(APPEND dep "BinaryStream" )
qgs_make_shared_lib( DataMsg src dep)

list(APPEND src_DataMsgTest "DataMsgTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "DataMsg")

qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
