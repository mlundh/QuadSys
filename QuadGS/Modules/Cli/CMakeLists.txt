list(APPEND src "CLI.cpp")
list(APPEND dep "AppLog"
                "readline"
                "Ui_Command"
                "Ui_CommandSlave"
                "Ui_CommandMaster"
                "QGS_Module")
qgs_make_shared_lib( cli src dep)
