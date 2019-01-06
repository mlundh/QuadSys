list(APPEND src "CLI.cpp")
list(APPEND dep "AppLog"
                "readline"
                "Ui_Command"
                "QGS_Module")
qgs_make_shared_lib( cli src dep)
