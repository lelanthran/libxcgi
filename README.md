# libxcgi - C library to support Common Gateway Interface programs

## Note: this library is not yet usable, still in development.

## Dependencies
1. Download the Data Structures library **libds**
   [from github](https://github.com/lelanthran/libds).

2. Compile **libds** and make sure that the resulting library files `libds.so`
   and `libds.a` are placed in your library path. By default this
   Makefile adds `$HOME/lib` (on POSIX systems) or `%HOMEDIR%\lib` (on Windows
   systems) to the list of paths to search for libraries.

3. Copy the include file `libxcgi.h` to the include path. By default this
   Makefile adds `$HOME/include` (on POSIX systems) or `%HOMEDIR%\include`
   (on Windows systems) to the include search path.


