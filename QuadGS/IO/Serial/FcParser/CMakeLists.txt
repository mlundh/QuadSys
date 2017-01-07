list(APPEND src "FcParser.cpp" )

list(APPEND dep "DataMsg"
                "BinaryStream")

qgs_make_shared_lib( FcParser src dep)
