list(APPEND src "CLI.cpp")
list(APPEND dep "AppLog"
                "readline"
                "GS_Ui_Command"
                "GS_FC_Status"
                "QGS_Module")
qgs_make_shared_lib( cli src dep)
