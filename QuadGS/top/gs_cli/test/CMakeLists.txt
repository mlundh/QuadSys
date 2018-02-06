
find_program (EXPECT_PROGRAM expect REQUIRED)

if (EXPECT_PROGRAM)
  message(STATUS "QuadGS system test availible. $<TARGET_FILE:QuadGS>")
  add_test (NAME testGui 
            COMMAND ${EXPECT_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/testGUI $<TARGET_FILE:QuadGS> )
else()
     message(STATUS "QuadGS system test not availible.")
endif (EXPECT_PROGRAM)
