list(APPEND src "QGS_Tree.cpp")
list(APPEND dep "AppLog")
qgs_make_shared_lib( CommandTree src dep)
