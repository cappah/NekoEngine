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

	PACKAGES="gcc make cmake sqlite openal libpng zlib libvorbis libx11 libbsd mesa-libgl"

	if ! type sudo 2> /dev/null; then
		su -c "pacman --noconfirm -Syy $PACKAGES"		
	else
		sudo pacman --noconfirm -Syy $PACKAGES
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed."
}

InstallDepsAptGet()
{
	echo "Attempting to install dependencies using apt-get"

	# Travis uses an ancient version of ubuntu that has an incomplete libbsd
	VER=`lsb_release -r | awk '{print $2}'`
	
	if [ "$VER" = "14.04" ]; then
		PACKAGES="build-essential cmake libsqlite3-dev libpng-dev libx11-dev libopenal-dev libvorbis-dev libgl1-mesa-dev"

		# Manually install libbsd
		echo "Ubuntu 14.04 detected, installing libbsd from source. Please upgrade your OS"
		wget --no-check-certificate https://libbsd.freedesktop.org/releases/libbsd-0.8.3.tar.xz;
		tar xf libbsd-0.8.3.tar.xz;
		cd libbsd-0.8.3;
		./configure --prefix=/usr;
		make;
		sudo make install;
		cd ..;
		echo "Done"
	else
		PACKAGES="build-essential cmake libsqlite3-dev libpng-dev libx11-dev libopenal-dev libvorbis-dev libgl1-mesa-dev libbsd-dev"
	fi


	if ! type sudo 2> /dev/null; then
		su -c "apt-get -y install $PACKAGES"		
	else
		sudo apt-get -y install $PACKAGES
	fi

	if [ $? -ne 0 ]; then
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

InstallDepsEquo()
{
	echo "Attempting to install dependencies using equo"
	PACKAGES="gcc make cmake sqlite libpng libX11 openal libvorbis libogg libGLw libbsd"

	if ! type sudo 2> /dev/null; then
		su -c "equo install $PACKAGES"		
	else
		sudo equo install $PACKAGES
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
		if [ "$1" == "release" ]; then
			BUILD=RELEASE
		else
			BUILD=DEBUG
		fi
	fi

	cmake -DCMAKE_BUILD_TYPE=$BUILD ..

	echo ""

	if [ $? -eq 0 ]; then
		echo "Setup done. Now cd build and run make to build the engine."
	else
		echo "Failed to generate makefiles."
	fi
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
		elif type equo 2> /dev/null; then
			InstallDepsEquo
		else
			echo "ERROR: Unknown distribution. You will have to install the dependencies manually."
			exit;
		fi

		GenerateX11Icon
		GenerateMakefile
	;;
	'FreeBSD')
		echo "Attempting to install dependencies using pkg"
		sudo pkg install -y llvm38 gmake cmake sqlite3 png libX11 openal-soft libvorbis libGL;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		CC=clang38
		CXX=clang++38

		GenerateX11Icon
		GenerateMakefile
	;;
	'SunOS')
		if ! type cmake 2>/dev/null; then
			if ! type pkgutil  2>/dev/null; then
				echo "CMake not found and pkgutil is unavailable. Please install CMake or OpenCSW";
				exit;
			fi

			echo "Attempting to install CMake using pkgutil"
			sudo /opt/csw/bin/pkgutil -y -i cmake;
		fi

		if ! type g++-5.2 2>/dev/null; then
			if type g++ 2>/dev/null; then
				echo "WARNING: Using system provided gcc. The build WILL FAIL with gcc 4.8 on Solaris 11"
				echo "Continiue only if you are on OpenIndiana 2016.04 or newer"
				echo "Type yes and press Enter to continue."	
				read response	
				case $response in
					'yes')
					echo "Continuing on user action"
					;;	
					*)
					echo "Aborting because of user action"
					exit;
					;;
				esac
			else
				if ! type pkgutil 2>/dev/null; then
					echo "GCC not found and pkgutil is unavailable. Please install CMake or OpenCSW";
					exit;
				fi

				echo "Attempting to install gcc5 using pkgutil"
				sudo /opt/csw/bin/pkgutil -y -i gcc5core gcc5g++;
			fi
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
		brew install libpng libvorbis freetype;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		echo ""
		echo "Setup done. Opening the Xcode workspace file..."
		open "Neko Engine.xcworkspace"
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
