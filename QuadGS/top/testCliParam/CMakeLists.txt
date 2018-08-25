list(APPEND src_CliParamTest   "testCLIParam.cpp")
list(APPEND dep_CliParamTest   "Parameters"
                               "cli"
                               "LogHandler")

qgs_add_test(cliParamTest src_CliParamTest dep_CliParamTest)