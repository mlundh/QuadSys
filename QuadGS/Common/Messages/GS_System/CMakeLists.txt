list(APPEND src "Msg_Stop.cpp"
				"Msg_RegisterName.cpp"
				"Msg_Display.cpp"
				"Msg_Test.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( GS_System src dep)

