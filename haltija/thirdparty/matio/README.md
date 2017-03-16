### Status (Mac Clang)
[![Build Status](https://travis-ci.org/audiofilter/matio.png)](https://travis-ci.org/audiofilter/matio)

From MATIO 1.5.2 - MATLAB MAT file I/O library

Modified for CMake build on Mac OSx 10.9
(This version was pre-configured for Mac OSX 10.9)
Modify src/matioConfig.h for different compile/link, etc options
(please see matio documentation)

####Needed Dependencies
cmake
hdf5

See MATIO 1.5.2 README from sourceforge

To build
  (brew install cmake hdf5)
  mkdir build; 
  cd build
  cmake ..
  make 

