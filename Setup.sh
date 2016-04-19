#!/bin/sh

HAVE_X11=NO
CC=gcc
CXX=g++

#############
# Functions #
#############

InstallDepsFail()
{
	echo "Failed to install dependencies. You will have to install them manually"
	exit -1;
}

InstallDepsPacman()
{
	echo "Attempting to install dependencies using pacman"
	sudo pacman -Syy gcc make cmake sqlite openal libpng zlib libvorbis libx11;

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
}

InstallDepsAptGet()
{
	echo "Attempting to install dependencies using apt-get"
	sudo apt-get update;
	sudo apt-get install build-essential cmake libsqlite3-dev libpng-dev libx11-dev libopenal-dev libvorbis-dev libgl1-mesa-dev; 

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
}

InstallDepsDnf()
{
	echo "Attempting to install dependencies using dnf"
	sudo dnf -y install gcc gcc-c++ make cmake sqlite-devel libpng-devel libX11-devel openal-devel libvorbis-devel mesa-libGL-devel;

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
}

InstallDepsYum()
{
	echo "Attempting to install dependencies using yum"
	sudo yum -y install gcc gcc-c++ make cmake sqlite-devel libpng-devel libX11-devel openal-devel libvorbis-devel mesa-libGL-devel;

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
}

########
# Main #
########

# Install dependencies
if [ `uname` == "Linux" ]; then
	HAVE_X11=YES

	# Search for package manager
	if hash pacman 2>/dev/null; then	
		InstallDepsPacman
	elif hash apt-get 2>/dev/null; then
		InstallDepsAptGet
	elif hash dnf 2>/dev/null; then
		InstallDepsDnf
	elif hash yum 2>/dev/null; then
		InstallDepsYum
	else
		echo "ERROR: Unknown distribution. You will have to install the dependencies manually."
		exit;
	fi
elif [ `uname` == "FreeBSD" ]; then
	HAVE_X11=YES

	echo "Attempting to install dependencies using pkg"
	sudo pkg install gcc5 gmake cmake sqlite3 png libX11 openal-soft libvorbis libGL;

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	CC=gcc5
	CXX=g++5

	echo "Dependencies installed"
elif [ `uname` == "SunOS" ]; then
	HAVE_X11=YES

	if [ hash cmake 2>/dev/null ]; then
		if [ hash pkgutil 2>/dev/null ]; then
			echo "CMake not found and pkgutil is unavailable. Please install CMake or OpenCSW";
			exit;
		fi

		echo "Attempting to install CMake using pkgutil"
		sudo /opt/csw/bin/pkgutil -y -i cmake;
	fi

	if [ hash g++ 2>/dev/null ]; then
		if [ hash pkgutil 2>/dev/null ]; then
			echo "GCC not found and pkgutil is unavailable. Please install CMake or OpenCSW";
			exit;
		fi

		echo "Attempting to install gcc5 using pkgutil"
		sudo /opt/csw/bin/pkgutil -y -i gcc5core gcc5g++;
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
elif [ `uname` == "Darwin" ]; then
	if [ hash brew 2>/dev/null ]; then
		echo "Homebrew not found. Attempting to install..."
		/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
		if [[ $? -ne 0 ]]; then
			InstallDepsFail
		fi
	fi

	brew update;
	brew install libpng libvorbis;

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi
fi

if [ $HAVE_X11 == YES ]; then

echo "Generating x11_icon.h"

# Build tools
mkdir -p Tools/bin
$CC -I/usr/local/include -L/usr/local/lib -o Tools/bin/png2argb Tools/png2argb.c -lpng

# Create x11_icon.h
Tools/bin/png2argb Launcher/X11/icon_16.png Launcher/X11/icon_32.png Launcher/X11/icon_64.png Launcher/X11/icon_128.png Launcher/X11/icon_256.png Launcher/X11/icon_512.png > Engine/Platform/X11/x11_icon.h

echo "Done"

fi

echo "Creatting build directory"
mkdir -p build

echo "Generating Makefile"
cd build

if [ $# -gt 0 ]; then
	if [ $1 == "release" ]; then
		cmake -DCMAKE_BUILD_TYPE=RELEASE ..
	else
		cmake -DCMAKE_BUILD_TYPE=DEBUG ..
	fi
fi

echo ""
echo "Setup done. Now cd build and run make to build the engine."
