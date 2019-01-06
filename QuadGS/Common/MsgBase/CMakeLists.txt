list(APPEND src "QGS_Msg.cpp"
                "QGS_ModuleMsg.cpp")
                
list(APPEND dep "BinaryStream"
				"AppLog")
				
qgs_make_shared_lib( MsgBase src dep)

list(APPEND src_DataMsgTest "QGS_ModuleMsgTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "MsgBase"
                            "GS_System")

qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
