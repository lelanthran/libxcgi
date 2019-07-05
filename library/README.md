# libxcgi - C library to support Common Gateway Interface programs

## Note: this library is not yet usable, still in development.

## Dependencies
1. Download the Data Structures library **libds**, minimum verison v1.0.1
   [from github](https://github.com/lelanthran/libds).

2. Compile **libds** and make sure that the resulting library files `libds.so`
   and `libds.a` are placed in your library path. By default this
   Makefile adds `$HOME/lib` (on POSIX systems) or `%HOMEDIR%\lib` (on Windows
   systems) to the list of paths to search for libraries so it is
   sufficient to place the `libsqldb.so` libraries in these folders.

3. Ensure that the **libds** headers are in an include path. By default
   this Makefile adds `$HOME/include` (on POSIX systems) or
   `%HOMEDIR%\include` (on Windows systems) to the include paths for the
   compiler.
   It is sufficient to simply copy the `include/*` files that are generated
   during the **libds** build to `$HOME/include` or `%HOMEDIR%\include`.

4. Download the SQL db wrapper library **libsqldb** minimum version v0.1.4
   [from github](https://github.com/lelanthran/libsqldb).

5. Compile **libsqldb** and make sure that the resulting library files
   `libsqldb.so` and `libsqldb.a` are placed in your library path. By
   default this Makefile adds `$HOME/lib` (on POSIX systems) or
   `%HOMEDIR%\lib` (on Windows systems) to the list of paths to search
   for libraries so it is sufficient to place the `libsqldb.so` libraries
   in these folders.

6. Ensure that the **libsqldb** headers are in an include path. By default
   this Makefile adds `$HOME/include` (on POSIX systems) or
   `%HOMEDIR%\include` (on Windows systems) to the include paths for the
   compiler.
   It is sufficient to simply copy the `include/*` files that are generated
   during the **libsqldb** build to `$HOME/include` or `%HOMEDIR%\include`.

7. Copy the include file `libxcgi.h` to the include path. By default this
   Makefile adds `$HOME/include` (on POSIX systems) or `%HOMEDIR%\include`
   (on Windows systems) to the include search path.


