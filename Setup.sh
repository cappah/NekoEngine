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
}

InstallDepsPacman()
{
	echo "Attempting to install dependencies using pacman"

	PACKAGES="gcc make cmake sqlite openal libvorbis zlib libx11 libbsd vulkan-headers vulkan-icd-loader vulkan-validation-layers vulkan-extra-layers vulkan-trace freetype2 luajit"

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
	
	PACKAGES="build-essential cmake libsqlite3-dev libx11-dev libvorbis-dev libopenal-dev libvorbis-dev libvulkan-dev libbsd-dev libfreetype6-dev libluajit-5.1-dev libbz2-dev libbullet-dev libbullet-extras-dev"

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
	PACKAGES="gcc gcc-c++ make cmake sqlite-devel libvorbis-devel libX11-devel openal-devel vulkan-devel libbsd-devel freetype-devel luajit-devel"

	if ! type sudo 2> /dev/null; then
		su -c "dnf -y install $PACKAGES"		
	else
		sudo dnf -y install $PACKAGES
	fi

	if [ $? -ne 0 ]; then
		InstallDepsFail
	fi

	echo "Dependencies installed. You will need to install or compile glslangValidator manually."
}

InstallDepsEquo()
{
	echo "Attempting to install dependencies using equo"
	PACKAGES="gcc make cmake sqlite libpng libX11 openal libGLw libbsd freetype"

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
	Tools/bin/png2argb Source/Launcher/X11/icon_16.png Source/Launcher/X11/icon_32.png Source/Launcher/X11/icon_64.png Source/Launcher/X11/icon_128.png Source/Launcher/X11/icon_256.png Source/Launcher/X11/icon_512.png > Source/Engine/Platform/X11/x11_icon.h

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
		echo "WARNING: There is no official support for Vulkan on FreeBSD. 3D graphics will most likely not work."
		echo "WARNING: FreeBSD is supported as a server platform only"

		read -p "Continue ? " -n 1 -r
		echo
		if [ ! $REPLY =! ^[Yy]$ ]
		then
			exit 1;
		fi

		echo "Attempting to install dependencies using pkg"
		sudo pkg install -y llvm38 gmake cmake sqlite3 png libX11 openal-soft freetype2;

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		CC=clang38
		CXX=clang++38

		GenerateX11Icon
		GenerateMakefile
	;;
	'SunOS')
		echo "WARNING: There is no official support for Vulkan on Solaris. 3D graphics will most likely not work"
		echo "WARNING: Solaris is supported as a server platform only"

		read -p "Continue ? " -n 1 -r
		echo
		if [ ! $REPLY =! ^[Yy]$ ]
		then
			exit 1;
		fi

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

				echo "Attempting to install gcc5 and freetype using pkgutil"
				sudo /opt/csw/bin/pkgutil -y -i gcc5core gcc5g++ libfreetype6 libfreetype_dev;
			fi
		fi

		if [ $? -ne 0 ]; then
			InstallDepsFail
		fi

		GenerateX11Icon
		GenerateMakefile
	;;
	'Darwin')
		echo "WARNING: There is no official support for Vulkan on Darwin. 3D graphics will most likely not work."
		echo "WARNING: Darwin is supported as a server platform only"

		read -p "Continue ? " -n 1 -r
		echo
		if [ ! $REPLY =! ^[Yy]$ ]
		then
			exit 1;
		fi

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
		open Projects/Xcode/NekoEngine.xcworkspace
	;;
	'OpenBSD')
		echo "WARNING: There is no official support for Vulkan on OpenBSD. 3D graphics will most likely not work."
		echo "WARNING: OpenBSD is supported as a server platform only"

		read -p "Continue ? " -n 1 -r
		echo
		if [ ! $REPLY =! ^[Yy]$ ]
		then
			exit 1;
		fi

		echo "Attempting to install dependencies using pkg"
		sudo pkg_add gcc g++ gmake cmake png openal freetype;

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
