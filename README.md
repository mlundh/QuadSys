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
Linux (tested on Ubuntu 16.04 )

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
cmake -DWITH_VISION=[true/false] .. 
make
```

