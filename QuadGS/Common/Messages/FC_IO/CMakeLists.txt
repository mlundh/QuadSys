list(APPEND src "Msg_DebugIo.cpp"
				"Msg_ParamIo.cpp"
				"Msg_LogIo.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( FC_IO src dep)

