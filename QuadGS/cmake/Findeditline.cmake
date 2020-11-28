
# Tries to find editline headers and libraries
#
# Usage of this module as follows:
#
#     find_package(editline)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  EDITLINE_ROOT_DIR   Set this variable to the root installation of
#                     editline if the module has problems finding 
#                     the proper installation path.
#
# Variables defined by this module:
#
#  editline_FOUND              System has editline libs/headers
#  editline_LIBRARIES          The editline libraries
#  editline_INCLUDE_DIR        The location of editline headers

find_path(editline_INCLUDE_DIR
          NAMES editline.h
          HINTS /usr/local/include ${editline_ROOT_DIR}/include)

find_library(editline_LIBRARY
             NAMES editline
             HINTS /usr/local/lib ${editline_ROOT_DIR}/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(editline
                                  DEFAULT_MSG
                                  editline_LIBRARY
                                  editline_INCLUDE_DIR)

if(editline_FOUND)
  set(editline_LIBRARIES ${editline_LIBRARY})
  set(editline_INCLUDE_DIRS ${editline_INCLUDE_DIR})
endif()

mark_as_advanced(editline_ROOT_DIR
                 editline_LIBRARY
                 editline_INCLUDE_DIR)

