list(APPEND src "Msg_Transmission.cpp"
				"Msg_TestTransmission.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( IO src dep)

