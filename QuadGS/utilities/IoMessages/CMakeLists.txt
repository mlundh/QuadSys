list(APPEND src "QGS_IoHeader.cpp"
                "QGS_ParamMsg.cpp"
                "QGS_DebugMsg.cpp"
                "QGS_LogMsg.cpp")
                
list(APPEND dep "BinaryStream"
				"AppLog"
				"QGS_Module")
				
qgs_make_shared_lib( IoMessages src dep)

list(APPEND src_DataMsgTest "DataMsgTest.cpp")
list(APPEND dep_DataMsgTest "gtest_main"
                            "IoMessages")

qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
