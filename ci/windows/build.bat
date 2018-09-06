
echo "Build..."


if "%GENERATE_ARCH%"=="Win64" (
    ::note the space before Win64
    set ARCH= Win64
) else (
    ::set to empty string
    set ARCH=
)

if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2017" (
    set GENERATOR=Visual Studio 15 2017%ARCH%
)
if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2015" (
    set GENERATOR=Visual Studio 14 2015%ARCH%
)
echo "GENERATOR: %GENERATOR%"


mkdir "%SystemDrive%\build"
cd "%SystemDrive%\build"

cmake -G "%GENERATOR%" "%APPVEYOR_BUILD_FOLDER%"
cmake --build . --config "%CONFIGURATION%" --target ALL_BUILD
