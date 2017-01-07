
# Function to build and install a shared library in QuadGS
function(qgs_make_shared_lib lib_name lib_files lib_deps)
    # Define library. Only source files here!
    project(${lib_name} VERSION 0.1 LANGUAGES CXX)
    
    qgs_list_add_prefix(${lib_files} src/)
        
    add_library(${lib_name} SHARED
        ${${lib_files}})
    
    # Define headers for this library. PUBLIC headers are used for
    # compiling the library, and will be added to consumers' build
    # paths.
    target_include_directories(${lib_name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE src)
    
    # If we have compiler requirements for this library, list them
    # here
    #target_compile_features(lib
    #    PUBLIC cxx_auto_type
    #    PRIVATE cxx_variadic_templates)
    
    # Depend on a library that we defined in the top-level file
    target_link_libraries(${lib_name}
        ${${lib_deps}})
    
    
    #Useful debugging commands
    #cmake_print_variables(lib_name ${lib_deps} ${lib_files})
    #cmake_print_properties(TARGETS ${lib_name} PROPERTIES
    #            INTERFACE_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)

    # 'make install' to the correct location
    install(TARGETS ${lib_name}
        ARCHIVE  DESTINATION lib
        LIBRARY  DESTINATION lib
        RUNTIME  DESTINATION bin)
    
    # Print stuff during installation
    install(CODE "MESSAGE(\"Sample install message.\")")
    

endfunction()

# Add a test to the QuadGS project
function(qgs_add_test test_name test_files test_deps)
    if(BUILD_TESTS)
            # Every library has unit tests, of course
        add_executable(${test_name}
            ${${test_files}})
    
        target_include_directories(${test_name} PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            PRIVATE test)
        
        target_link_libraries(${test_name}
            ${${test_deps}})
    
        add_test(${test_name} ${test_name})
    endif()
endfunction()

# Add a executable to the QuadGS project
function(qgs_add_exe exe_name exe_files exe_deps)

        # Every library has unit tests, of course
    add_executable(${exe_name}
        ${${exe_files}})

    target_include_directories(${exe_name} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE src)
    
    target_link_libraries(${exe_name}
        ${${exe_deps}})
        
    # 'make install' to the correct location
    install(TARGETS ${exe_name}
        ARCHIVE  DESTINATION lib
        LIBRARY  DESTINATION lib
        RUNTIME  DESTINATION bin)

endfunction()

# add prefix to each item in the list
macro(qgs_list_add_prefix LIST PREFIX)
  set(__tmp_list "")
  foreach(item ${${LIST}})
    list(APPEND __tmp_list "${PREFIX}${item}")
  endforeach()
  set(${LIST} ${__tmp_list})
  unset(__tmp_list)
endmacro()

