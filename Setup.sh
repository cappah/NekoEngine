#!/bin/sh

HAVE_X11=NO

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
#	pacman -Syy cmake sqlite openal libpng zlib libvorbis glm libx11;

#	if [[ $? -ne 0 ]]; then
#		InstallDepsFail
#	fi

	echo "Dependencies installed"
}

InstallDepsAptGet()
{
	echo "Attempting to install dependencies using apt-get"
	apt-get update;
	apt-get install cmake libsqlite3-dev libpng-dev libglm-dev libx11-dev libopenal-dev libvorbis-dev libgles2-mesa-dev libgl1-mesa-dev; 

	if [[ $? -ne 0 ]]; then
		InstallDepsFail
	fi

	echo "Dependencies installed"
}

InstallDepsDnf()
{
	echo "pacman"
}

InstallDepsYum()
{
	echo "pacman"
}

########
# Main #
########

# Install dependencies
if [[ `uname` == "Linux" ]]; then
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
elif [[ `uname` == "FreeBSD" ]]; then
	HAVE_X11=YES
elif [[ `uname` == "SunOS" ]]; then
	HAVE_X11=YES
elif [[ `uname` == "Darwin" ]]; then
	if hash brew 2>/dev/null; then
		echo "Homebrew not found. Attempting to install..."
		/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
		if [[ $? -ne 0 ]]; then
			InstallDepsFail
		fi
	fi

	brew update;
	brew install libpng libvorbis glm;

	if [[ $? -ne 0 ]]; then
		InstallDepsFail
	fi
fi

if [[ $HAVE_X11 == YES ]]; then

echo "Generating x11_icon.h"

# Build tools
mkdir -p Tools/bin
cc -I/usr/local/include -L/usr/local/lib -o Tools/bin/png2argb Tools/png2argb.c -lpng

# Create x11_icon.h
Tools/bin/png2argb Launcher/X11/icon_16.png Launcher/X11/icon_32.png Launcher/X11/icon_64.png Launcher/X11/icon_128.png Launcher/X11/icon_256.png Launcher/X11/icon_512.png > Engine/Platform/X11/x11_icon.h

echo "Done"

fi

echo "Creatting build directory"
mkdir -p build

echo "Generating Makefile"
cd build
if [[ $1 == "release" ]]; then
	cmake -DCMAKE_BUILD_TYPE=RELEASE ..
else
	cmake -DCMAKE_BUILD_TYPE=DEBUG ..
fi

echo ""
echo "Setup done. Now cd build and run make to build the engine."
