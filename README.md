## DDFA - Dynamic Data Flow Analysis Library

### Build and install the library

1. Clone this repo

		git clone https://github.com/passlab/DDFA.git
		
2. Build and install the DDFA library. You can build and install in the directory you want. Commands in the following are for
   building and installing in the `build` and `install` folder under the source tree. 

		cd DDFA
		mkdir build install
		cd build
		cmake -DCMAKE_INSTALL_PREFIX=../install ..
		make
		make install

### Run examples

##### square.c for simple function call profiling

		cd ../examples
		make
		export LD_LIBRARY_PATH=../install/lib
		./square
		
### Acknowledgement and Citation
Funding for this research and development was provided by the National Science Foundation 
under award No. 2001580 and 2015254. 
