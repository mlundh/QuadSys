list(APPEND src "Log.cpp")
list(APPEND dep ${Boost_LIBRARIES})
qgs_make_shared_lib( log src dep)
