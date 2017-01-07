list(APPEND src "BinaryStream.cpp")
list(APPEND dep "")
qgs_make_shared_lib( BinaryStream src dep)


list(APPEND src_BinaryStreamTest "test/BinaryStreamTest.cpp")
list(APPEND dep_BinaryStreamTest "gtest_main"
                                 "BinaryStream")

qgs_add_test(BinaryStreamTest src_BinaryStreamTest dep_BinaryStreamTest)
