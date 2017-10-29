#!/bin/bash

OUTPUT_DIRECTORY=../../../Resources/Data/Shaders
SHC=glslc
SHCFLAGS="--target-env=vulkan -Os -I../include"

compileShader()
{
	OUT="${1/$2/.spv}"
	$SHC $SHCFLAGS $1 -o $OUTPUT_DIRECTORY/$OUT
}

echo Compiling vertex shaders...
cd vertex
for file in *.vert
do
	compileShader $file .vert
done
cd ..

#cd tesc
#echo Compiling tesselation control shaders...
#for file in *.tesc
#do
#	compileShader $file .tesc
#done
#cd ..

#cd tese
#echo Compiling tesselation eval shaders...
#for file in *.tese
#do
#	compileShader $file .tese	
#done
#cd ..

cd geometry
echo Compiling geometry shaders...
for file in *.geom
do
	compileShader $file .geom
done
cd ..

cd fragment
echo Compiling fragment shaders...
for file in *.frag
do
	compileShader $file .frag
done
cd ..

cd pp
echo Compiling post process shaders...
for file in *.frag
do
	compileShader $file .frag
done
cd ..

cd compute
echo Compiling compute shaders...
for file in *.comp
do
	compileShader $file .comp
done
cd ..

