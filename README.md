# reflex-bison-ast

Cloned from https://github.com/UO-cis561/reflex-bison-ast. Modified to parse Quack (.qk) source code. 
Successfully parsed files will print the message "Finished parse with no errors". Otherwise, information about the location of an error, the encountered symbol, and the expected token will be printed followed by "Unable to parse!". 

## Building
Run the following commands from the project's root directory:
```
mkdir cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
