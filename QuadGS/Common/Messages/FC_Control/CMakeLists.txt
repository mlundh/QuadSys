list(APPEND src "Msg_FlightModeReq.cpp"
				"Msg_CtrlModeReq.cpp"
				"Msg_NewSetpoint.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( FC_Control src dep)

