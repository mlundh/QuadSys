# QuadSys 
QuadSys consists of two parts, one MCU part that contains the flight controller and all tests related to it, and the linux part QuadGS that is used for configuring the FC. 


# QuadFC

is a flight controller for quadcopters. It is designed to be easy to port to any mcu that can run FreeRTOS, but the only port currently availible is for the arduino due with certain peripherals. It is also designed for modularity, and the modules themselfs can easily be used for someting other than quadcopter control. 

# QuadGS

Ground station to configure and log data from QuadFC. A serial connection should be established to QuadFC. 

### Program arguments: 

```  -h [ --help ]                  produce help message
  -v [ --version ]               print version string
  -g [ --gui ]                   Gui mode
  -l [ --LogLevel ] arg (=error) Log level for outputing to screen. {error, warning, info, debug}
```
Quit the program by typing "quit" and press enter. 

### Dependencies
  
Other:
 - boost
 - readline
 - gtest (automatically downloaded by cmake)
 - ZED sdk (optional, required if WITH_VISION=true)
 - openCV (optional, required if WITH_VISION=true)
  
##### tested on:
Linux (tested on Ubuntu 16.04, 19.04 )

### Other
- Set the following env variables to build: 

``` 
BOOST_ROOT = path_to_boost_root_dir
``` 
Linux:
Install libreadline-dev. 

Build: 
```
mkdir build
cd build
cmake -DWITH_VISION=[true/false] -DBoost_NO_BOOST_CMAKE=ON.. 
make
```

