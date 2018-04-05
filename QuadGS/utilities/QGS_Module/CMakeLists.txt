list(APPEND src "QGS_Module.cpp"
                "QGS_Router.cpp"
                "QGS_Msg.cpp"
                "QGS_ModuleMsg.cpp"
                "QGS_CommandMsg.cpp")
                
list(APPEND dep "AppLog"
                "Fifo"
                "BinaryStream")

qgs_make_shared_lib( QGS_Module src dep)

list(APPEND src_Test "QGS_ModuleTest.cpp"
                     "QGS_ModuleFake.cpp"
                     "QGS_ModuleMsgTest.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module"
                     "IoMessages")

qgs_add_test(QGS_ModuleTest src_Test dep_Test)