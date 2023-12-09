set PLATFORM=x64
set TARGET=ACCAT_opengl
set BUILD_PATH=%TARGET%\%PLATFORM%

cd %~dp0
mkdir %BUILD_PATH%
cd %BUILD_PATH%
cmake -D CMAKE_CONFIGURATION_TYPES="Debug;Release" -G "Visual Studio 17 2022" -A x64 -S %~dp0 -B .

pause