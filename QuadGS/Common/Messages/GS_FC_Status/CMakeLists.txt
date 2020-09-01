list(APPEND src "Msg_FlightMode.cpp"
				"Msg_CtrlMode.cpp"
				"Msg_FcFault.cpp"
				"Msg_Error.cpp"
				"Msg_BindRc1.cpp"
				"Msg_BindRc2.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( GS_FC_Status src dep)

