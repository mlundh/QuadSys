list(APPEND src "QGS_Tree.cpp")
list(APPEND dep "log")
qgs_make_shared_lib( CommandTree src dep)
