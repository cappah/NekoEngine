@ECHO OFF

set OUTPUT_DIRECTORY=..\..\Resources\Data\Shaders
set SHC="%VK_SDK_PATH%\Bin\glslc.exe"
set SHCFLAGS=--target-env=vulkan -Iinclude
mkdir %OUTPUT_DIRECTORY%

echo.
echo Compiling vertex shaders...
for /f %%f in ('dir /b vertex') do (call :compile_shader %%f .vert vertex)

echo.
echo Compiling tesselation control shaders...
for /f %%f in ('dir /b tesc') do (call :compile_shader %%f .tesc tesc)

echo.
echo Compiling tesselation eval shaders...
for /f %%f in ('dir /b tese') do (call :compile_shader %%f .tese tese)

echo.
echo Compiling geometry shaders...
for /f %%f in ('dir /b geometry') do (call :compile_shader %%f .geom geometry)

echo.
echo Compiling fragment shaders...
for /f %%f in ('dir /b fragment') do (call :compile_shader %%f .frag fragment)

echo.
echo Compiling post process shaders...
for /f %%f in ('dir /b pp') do (call :compile_shader %%f .frag pp)

echo.
echo Compiling compute shaders...
for /f %%f in ('dir /b compute') do (call :compile_shader %%f .comp compute)

goto :eof

:compile_shader
	SETLOCAL ENABLEDELAYEDEXPANSION
	set src_file=%1
	set out_file=!src_file:%2=.spv!
	%SHC% %SHCFLAGS% %3\%src_file% -o %OUTPUT_DIRECTORY%\%out_file%
	ENDLOCAL
GOTO :eof

pause
