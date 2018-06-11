list(APPEND src "QGS_IoHeader.cpp"
                "QGS_ParamMsg.cpp"
                "QGS_DebugMsg.cpp"
                "QGS_LogMsg.cpp"
                "QGS_Msg.cpp"
                "QGS_ModuleMsg.cpp")
                
list(APPEND dep "BinaryStream"
				"AppLog")
				
qgs_make_shared_lib( MsgBase src dep)

list(APPEND src_DataMsgTest "DataMsgTest.cpp"
                            "QGS_ModuleMsgTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "IoMessages")

qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
