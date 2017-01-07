set(src "Parameters.cpp")
set(dep "CommandTree"
        "DataMsg"
        "log")

qgs_make_shared_lib( Parameters src dep)


list(APPEND src_ParametersTest   "test/ParametersTest.cpp")
list(APPEND dep_ParametersTest   "gtest_main"
                                 "Parameters")

qgs_add_test(ParametersTest src_ParametersTest dep_ParametersTest)