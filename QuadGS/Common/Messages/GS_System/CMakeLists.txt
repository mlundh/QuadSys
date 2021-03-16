list(APPEND src "Msg_Stop.cpp"
				"Msg_RegisterName.cpp"
				"Msg_Display.cpp"
				"Msg_GsAppLog.cpp"
				"Msg_GsHasLog.cpp"
				"Msg_GsLogLevel.cpp"
				"Msg_Test.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_System src dep)

