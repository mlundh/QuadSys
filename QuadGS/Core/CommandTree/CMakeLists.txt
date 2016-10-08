list(APPEND src "QuadGSTree.cpp")
list(APPEND dep "log")
qgs_make_shared_lib( CommandTree src dep)
