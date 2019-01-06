# - Find the readline library
# This module defines
#  READLINE_INCLUDE_DIR, path to readline/readline.h, etc.
#  READLINE_LIBRARY, the libraries required to use READLINE.
#  READLINE_FOUND, If false, do not try to use READLINE.
# also defined, but not for general use are

if(DEFINED ENV{READLINE_ROOT})
  set(READLINE_ROOT $ENV{READLINE_ROOT})
  message("Got a readline root: " ${READLINE_ROOT})
endif()


IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_path(READLINE_ROOT
    NAMES include/readline/readline.h
    )
endif()
  
# Apple readline does not support readline hooks
# So we look for another one by default
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h PATHS
    ${READLINE_ROOT}/include
    /usr/local/opt/readline/include
    /sw/include
    /opt/local/include
    /opt/include
    /usr/local/include
    /usr/include/
    NO_DEFAULT_PATH
    )
endif()
find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h)


# Apple readline does not support readline hooks
# So we look for another one by default
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_library(READLINE_readline_LIBRARY NAMES readline PATHS
     ${READLINE_ROOT}/lib
    /sw/lib
    /opt/local/lib
    /opt/lib
    /usr/local/lib
    /usr/lib
    NO_DEFAULT_PATH
    )
endif()
find_library(READLINE_readline_LIBRARY NAMES readline)

mark_as_advanced(
  READLINE_INCLUDE_DIR
  READLINE_readline_LIBRARY
  )

set( READLINE_FOUND "NO" )
if(READLINE_INCLUDE_DIR)
  if(READLINE_readline_LIBRARY)
    set( READLINE_FOUND "YES" )
    set( READLINE_LIBRARY ${READLINE_readline_LIBRARY})
    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Readline DEFAULT_MSG READLINE_INCLUDE_DIR READLINE_LIBRARY )
  endif()
endif()
