list(APPEND src "Msg_Debug.cpp"
				"Msg_TestMem.cpp"
				"Msg_TestMemReg.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( GS_FC_Debug src dep)

