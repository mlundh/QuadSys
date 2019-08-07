list(APPEND src "QGS_Module.cpp"
                "QGS_Router.cpp")
                
list(APPEND dep "AppLog"
                "Fifo"
                "BinaryStream"
                "GS_System")

qgs_make_shared_lib( QGS_Module src dep)

list(APPEND src_Test "QGS_ModuleTest.cpp"
                     "QGS_ModuleFake.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module"
                     "GS_FC_Param"
                     "GS_FC_Log")

qgs_add_test(QGS_ModuleTest src_Test dep_Test)