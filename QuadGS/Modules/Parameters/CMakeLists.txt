set(src "Parameters.cpp")
set(dep "CommandTree"
        "GS_FC_Param"
        "GS_Ui_Command"
        "QGS_Module")

qgs_make_shared_lib( Parameters src dep)


list(APPEND src_ParametersTest   "ParametersTest.cpp")
list(APPEND dep_ParametersTest   "gtest_main"
                                 "Parameters")

qgs_add_test(ParametersTest src_ParametersTest dep_ParametersTest)