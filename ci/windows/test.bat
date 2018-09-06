
echo "Test..."

cd "%SystemDrive%\build"
ctest --verbose --build-config "%CONFIGURATION%"
