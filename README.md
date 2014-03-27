GCE
=======

GCE is an actor model implementation featuring lightweight & fast
actor implementations, type matching for messages,
network transparent messaging, and more.

Get the Sources
---------------

* git clone git://github.com/nousxiong/gce.git

or

* svn checkout https://github.com/nousxiong/gce/trunk

Dependencies
------------

* CMake
* Boost 1.55.0 and newer

Need build sub librares:

* Boost.Atomic 
* Boost.Coroutine 
* Boost.Context 
* Boost.System 
* Boost.Regex 
* Boost.DateTime 
* Boost.Timer 
* Boost.Chrono 
* Boost.Thread

Please build boost with stage mode (for example: b2 ... stage)

Supported Compilers
-------------------

* GCC >= 4.6
* VC >= 9.0 (sp1)

Build (Linux)
-----------

* (cd to your gce sources root dir)
* cd ..
* mkdir gce_build
* cd gce_build
* cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg" ../gce
* make
* *Optional:* make install (if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=../install)

Build (Windows)
-----------

* (on cmd console, cd to your gce sources root dir)
* cd ..
* mkdir gce_build
* cd gce_build
* cmake -G "Visual Studio 9" -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg" ..\gce
* (open generated vc sln, and build ALL_BUILD project)
* *Optional:* (build INSTALL project)(if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=..\install)

