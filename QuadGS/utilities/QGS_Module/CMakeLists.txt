list(APPEND src "QGS_Module.cpp"
                "QGS_Router.cpp"
                "QGS_ModuleMsg.cpp")
                
list(APPEND dep "AppLog"
                "Fifo"
                "Messages")

qgs_make_shared_lib( QGS_Module src dep)

list(APPEND src_Test "QGS_ModuleTest.cpp"
                     "QGS_ModuleFake.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module")

qgs_add_test(QGSModuleTest src_Test dep_Test)