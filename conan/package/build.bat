@echo off

if exist build rmdir /s /q build
if exist debug rmdir /s /q debug
if exist release rmdir /s /q release

mkdir build\release32
mkdir build\debug32
mkdir build\release64
mkdir build\debug64

pushd build\release32
cmake ..\..\..\.. -A "Win32" -DCMAKE_BUILD_TYPE="RelWithDebInfo"
cmake --build . --config RelWithDebInfo
popd

pushd build\debug32
cmake ..\..\..\.. -A "Win32" -DCMAKE_BUILD_TYPE="Debug"
cmake --build . --config Debug
popd

pushd build\release64
cmake ..\..\..\.. -A "x64" -DCMAKE_BUILD_TYPE="RelWithDebInfo"
cmake --build . --config RelWithDebInfo
popd

pushd build\debug64
cmake ..\..\..\.. -A "x64" -DCMAKE_BUILD_TYPE="Debug"
cmake --build . --config Debug
popd

mkdir release\bin
mkdir debug\bin

copy build\release32\bin\soundtrack-plugin-32.dll release\bin
copy build\release32\bin\soundtrack-plugin-32.pdb release\bin
copy build\release64\bin\soundtrack-plugin-64.dll release\bin
copy build\release64\bin\soundtrack-plugin-64.pdb release\bin

copy build\debug32\bin\soundtrack-plugin-32.dll debug\bin
copy build\debug32\bin\soundtrack-plugin-32.pdb debug\bin
copy build\debug64\bin\soundtrack-plugin-64.dll debug\bin
copy build\debug64\bin\soundtrack-plugin-64.pdb debug\bin

goto end

:fail
echo --- failure ---
goto end

:end
