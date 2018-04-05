list(APPEND src "FcParser.cpp" )

list(APPEND dep "IoMessages"
                "BinaryStream")

qgs_make_shared_lib( FcParser src dep)
