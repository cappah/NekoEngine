NekoEngine
============

Cross-platform 3D Game Engine

**Requirements:**

       A PC with modern graphics card capable of OpenGL 4.5  (for Windows & *nix)
       - or -
       A Mac with OpenGL 4.1 support (anything from 2011 onwards should work; only for Mac OS)

**The engine has been tested on the following operating systems:**

* Windows 7 & 10
* Linux
* FreeBSD 10.2
* Solaris 11
* Mac OS X 10.11

Building
===============================

*nix
--------------------------

**Install dependencies**

Install the folowing libraries & their development files using your system's package manager, or compile them from source.

       sqlite3, X11, libpng, libvorbisfile, zlib, glm

You will also need the OpenGL headers and CMake.

**Build the engine**

1) Get the source

2) Run Setup.sh to generate the required files

2) Make a separate build directory

3) Run cmake & make

Example:

        git clone https://github.com/nalexandru/NekoEngine.git
        cd NekoEngine
        ./Setup.sh
        mkdir build && cd build
        cmake ..
        make


Windows
------------------------

**Install Visual Studio**

  The project file was created with Visual Studio 2015 Community Edition, which can be found here:
  https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
  
  You need to install at least Visual C++ & Windows SDK

**Build the engine**

  Note: The project file has only Win64 build configurations. If you want a 32-bit build you will need to create the build configurations & compile the dependencies.

  The solution file is located in the root directory. All necessary dependencies are in the 3rdparty directory.

Mac OS X
--------------------------

**Install Xcode**

  Install Xcode from the App Store.

**Install dependencies**

  Dependencies that don't ship with Xcode:
  
  * libpng
  * libvorbisfile
  * glm

  The dependencies can be installed using homebrew:
  
        brew install libpng libvorbis glm

  Alternatively, you can compile them with the prefix set to /usr/local.

**Build the engine**

  The Xcode workspace file is located in the root directory.

Running
===============================

*nix
--------------------------

Run the following command in the build directory:

       ./Launcher --ini=../Resources/Engine.ini --data=../Resources/Data --renderer=libGL4Renderer.so --game=libTestGame.so

Alternatively, you can copy the Engine.ini file from Resources/Engine.ini to the build directory and modify the following:

        sDataDirectory=../Resources/Data
        sRenderer=GL4Renderer -> sRenderer=libGL4Renderer.so
        sGameModule=TestGame -> sGameModule=libTestGame.so

Then run the launcher:

        ./Launcher

Windows
--------------------------

Go to the source directory with a file manager and:

1) Copy 3rd-party DLL's from 3rdparty/bin to Bin64

2) Copy or symlink Resources/Data & Resources/Engine.ini to Bin64

3) Start Launcher.exe, located in Bin64


Mac OS X
--------------------------

You should be able to start the program with Xcode.

To start without Xcode, until i make a proper .app bundle for the Launcher do the following:

1) Open a terminal and cd to the build directory (Right-click the Launcher executable in Xcode and select Show in Finder to find the build directory)

2) Run the following commands:

        export ENGINE_ROOT=<path to the engine source>
        ./Launcher --ini=$ENGINE_ROOT/Resources/Engine.ini --data=$ENGINE_ROOT/Resources/Data --renderer=libMacGLRenderer.dylib --game=libTestGame.dylib

Configuration
===============================

The settings which can be configured are in Engine.ini, located in the Resources folder.
