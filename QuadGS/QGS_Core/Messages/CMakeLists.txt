list(APPEND src "QGS_MsgHeader.cpp"
                "QGS_Msg.cpp"
                "QGS_ParamMsg.cpp"
                "QGS_DebugMsg.cpp"
                "QGS_LogMsg.cpp")
                
list(APPEND dep "BinaryStream" )
qgs_make_shared_lib( Messages src dep)

list(APPEND src_DataMsgTest "DataMsgTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "Messages")

qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
