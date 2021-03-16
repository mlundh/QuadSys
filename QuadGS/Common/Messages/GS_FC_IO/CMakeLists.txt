list(APPEND src "Msg_Transmission.cpp"
				"Msg_TestTransmission.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_FC_IO src dep)

