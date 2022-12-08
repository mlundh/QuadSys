list(APPEND src "Msg_FlightModeReq.cpp"
				"Msg_CtrlModeReq.cpp"
				"Msg_NewSetpoint.cpp"
				"Msg_NewRcSetpoint.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_FC_Control src dep)

