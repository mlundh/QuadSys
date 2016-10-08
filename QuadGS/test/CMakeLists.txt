
find_program (EXPECT_PROGRAM expect REQUIRED)

if (EXPECT_PROGRAM)
  message(STATUS "QuadGS system test availible.")
  add_test (testGui ${EXPECT_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/testGUI)
else()
     message(STATUS "QuadGS system test not availible.")
endif (EXPECT_PROGRAM)
