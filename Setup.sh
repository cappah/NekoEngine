#!/bin/sh

HAVE_X11=NO
CC=gcc
CXX=g++
SU=sudo

#############
# Functions #
#############

InstallDepsFail()
{
	echo "Failed to install dependencies. You will have to install them manually"
	exit;
}

InstallDepsPacman()
{
	echo "Attempting to install dependencies using pacman"

	PACKAGES="gcc make cmake sqlite openal libpng zlib libvorbis libx11"

	if ! type sudo 2> /dev/null; then
		su -c "pacman -Syy $PACKAGES"		
	else
		sudo pacman -Syy $PACKAGES
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed."
}

InstallDepsAptGet()
{
	echo "Attempting to install dependencies using apt-get"

	PACKAGES="build-essential cmake libsqlite3-dev libpng-dev libx11-dev libopenal-dev libvorbis-dev libgl1-mesa-dev libbsd-dev"

	if ! type sudo 2> /dev/null; then
		su -c "apt-get install $PACKAGES"		
	else
		sudo apt-get install $PACKAGES
	fi

	if $? -ne 0 ; then
		InstallDepsFail
	fi

	echo "Dependencies installed."
}

InstallDepsDnf()
{
	echo "Attempting to install dependencies using dnf"
	PACKAGES="gcc gcc-c++ make cmake sqlite-devel libpng-devel libX11-devel openal-devel libvorbis-devel mesa-libGL-devel libbsd-devel"

	if ! type sudo 2> /dev/null; then
		su -c "dnf -y install $PACKAGES"		
	else
		sudo dnf -y install $PACKAGES
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed."
}

InstallDepsYum()
{
	echo "Attempting to install dependencies using yum"
	PACKAGES="gcc gcc-c++ make cmake sqlite-devel libpng-devel libX11-devel openal-devel libvorbis-devel mesa-libGL-devel libbsd-devel"

	if ! type sudo 2> /dev/null; then
		su -c "yum -y install $PACKAGES"		
	else
		sudo yum -y install $PACKAGES
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed."
}

######################
# Generate functions #
######################

GenerateX11Icon()
{
	echo "Generating x11_icon.h"

	# Build tools
	mkdir -p Tools/bin
	$CC -I/usr/local/include -L/usr/local/lib -o Tools/bin/png2argb Tools/png2argb.c -lpng

	# Create x11_icon.h
	Tools/bin/png2argb Launcher/X11/icon_16.png Launcher/X11/icon_32.png Launcher/X11/icon_64.png Launcher/X11/icon_128.png Launcher/X11/icon_256.png Launcher/X11/icon_512.png > Engine/Platform/X11/x11_icon.h

	echo "Done"
}

GenerateMakefile()
{
	echo "Creating build directory"
	mkdir -p build

	echo "Generating Makefile"
	cd build

	BUILD=DEBUG

	if [ $# -gt 0 ]; then
		if $1 == "release" ; then
			BUILD=RELEASE
		else
			BUILD=DEBUG
		fi
	fi

	cmake -DCMAKE_BUILD_TYPE=$BUILD ..

	echo ""
	echo "Setup done. Now cd build and run make to build the engine."
}

########
# Main #
########

OS=`uname`
case $OS in
	'Linux')
		# Search for package manager
		if type pacman 2> /dev/null; then	
			InstallDepsPacman
		elif type apt-get 2> /dev/null; then
			InstallDepsAptGet
		elif type dnf 2> /dev/null; then
			InstallDepsDnf
		elif type yum 2> /dev/null; then
			InstallDepsYum
		else
			echo "ERROR: Unknown distribution. You will have to install the dependencies manually."
			exit;
		fi

		GenerateX11Icon
		GenerateMakefile
	;;
	'FreeBSD')
		echo "Attempting to install dependencies using pkg"
		sudo pkg install gcc5 gmake cmake sqlite3 png libX11 openal-soft libvorbis libGL;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		CC=gcc5
		CXX=g++5

		GenerateX11Icon
		GenerateMakefile
	;;
	'SunOS')
		if ! type hash cmake 2>/dev/null; then
			if [ hash pkgutil 2>/dev/null ]; then
				echo "CMake not found and pkgutil is unavailable. Please install CMake or OpenCSW";
				exit;
			fi

			echo "Attempting to install CMake using pkgutil"
			sudo /opt/csw/bin/pkgutil -y -i cmake;
		fi

		if ! type g++-5.2 2>/dev/null; then
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

		GenerateX11Icon
		GenerateMakefile
	;;
	'Darwin')
		if [ hash brew 2>/dev/null ]; then
			echo "Homebrew not found. Attempting to install..."
			/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
			if $? -ne 0 ; then
				InstallDepsFail
			fi
		fi

		brew update;
		brew install libpng libvorbis;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		echo ""
		echo "Setup done. Now open the Xcode workspace file."
	;;
	'OpenBSD')
		echo "Attempting to install dependencies using pkg"
		sudo pkg_add gcc g++ gmake cmake png openal libvorbis;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		CC=egcc
		CXX=eg++

		GenerateX11Icon
		GenerateMakefile
	;;
	*)
	;;
esac
