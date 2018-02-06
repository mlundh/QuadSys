list(APPEND src "AppLog.cpp")
list(APPEND dep ${Boost_LIBRARIES})
qgs_make_shared_lib( AppLog src dep)
