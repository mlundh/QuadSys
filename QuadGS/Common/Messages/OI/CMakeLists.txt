list(APPEND src "Msg_IoWrapper.cpp"
				"Msg_Transmission.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( OI src dep)

