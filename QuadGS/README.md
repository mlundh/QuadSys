QuadGS

Ground station to configure and log data from QuadFC. A serial connection should be established to QuadFC. 

Program arguments: 
  -h [ --help ]                  produce help message
  -v [ --version ]               print version string
  -g [ --gui ]                   Gui mode
  -l [ --LogLevel ] arg (=error) Log level for outputing to screen. {error, warning, info, debug}

Quit the program by typing "quit" and press enter. 

Dependencies
Boost libs: 
  boost_thread
  boost_program_options
  boost_system
  boost_log
  boost_log_setup
  
Other:
  readline

Mac and Linux (tested on Ubuntu 14.xx and OS X 10.10.3)

Build and install the boost libs acording to the boost dockumentation at http://www.boost.org . Make sure to build for dynamic linkage. 

Set the following env variables to build: 

BOOST_LIB_ROOT = path_to_boost_libs
BOOST_ROOT = path_to_boost_root_dir


Mac: 
Install readline lib using homebrew. Force linkage for compilaion.

If you still have problems linking the applikation try setting env variable:
AUX_LIB = path_to_readline

make sure that the loader knows where the libs are (might be a better way of dooing this): 

DYLD_LIBRARY_PATH=$AUX_LIB:$BOOST_LIB_ROOT


Linux:
Readline should be installed, but you might need the dev version of the libs. 

Add BOOST_LIB_ROOT to LD_LIBRARY_PATH.



Build: 
make

run:
./out/QuadGS