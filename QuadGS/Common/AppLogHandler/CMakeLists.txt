list(APPEND src "AppLogHandler.cpp")
list(APPEND dep "MsgBase"
                "GS_System"
                "QGS_Module"
                "GS_Ui_Command")
qgs_make_shared_lib( AppLogHandler src dep)