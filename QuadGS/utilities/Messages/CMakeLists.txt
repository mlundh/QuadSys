list(APPEND src "Msg_Debug.cpp"
                "Msg_Param.cpp"
                "Msg_Log.cpp"
                "Msg_Stop.cpp")
                
list(APPEND dep "BinaryStream"
				"IoMessages"
				"AppLog")
				
qgs_make_shared_lib( Messages src dep)

#list(APPEND src_DataMsgTest "DataMsgTest.cpp")
#list(APPEND dep_DataMsgTest "gtest_main"
#                            "IoMessages")

#qgs_add_test(DataMsgTest src_DataMsgTest dep_DataMsgTest)
