
You will need the .lib files from an OBS build to link against. 
Build OBS following the instructions here: https://obsproject.com/wiki/Install-Instructions#windows-build-directions

If building 32-bit vs 64-bit, be sure to set the the "deps" variables appropriately. E.g.:

32-bit:
set DepsPath32=c:\path\to\obs-deps\win32
set QTDIR=c:\path\to\obs-deps\qt\5.10.1\msvc2017
cmake -A Win32 -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build32

64-bit:
set DepsPath64=c:\path\to\obs-deps\win64
set QTDIR64=c:\path\to\obs-deps\qt\5.10.1\msvc2017_64
cmake -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build64

Open obs-studio.sln, select configuration type (e.g. RelWithDebInfo), and build. 
If it fails, double check the build process and build instructions for any problems.


After a successful build, go to your Soundtrack OBS repo cloned folder.
Create a symbolic link, or copy the generated files over to the plugin to consume them. 
The directory should be a direct link to the obs-studio repo. The build32 and build64 folders 
can also be symbolic linked separately if generated inside a different build folder.

mklink /D obs-studio c:\path\to\obs-studio

To build the actual Soundtrack OBS plugin, now open the project (either generate a solution 
from cmake, or use Visual Studio to open the cmake project by running `devenv .` or File>Open>CMake...)

Select the matching configuration type for the Soundtrack OBS plugin and build (note that 
"Release" in the Soundtrack OBS plugin project uses the "RelWithDebInfo" configuration type, 
so building a Release build may require that from OBS). Ideally, this should work, and generate 
some output files in a CMakeBuilds user folder, e.g.:
c:\Users\username\CMakeBuilds\01234567-89ab-cdef-fedc-ba9876543210\build\x86-Release\bin\soundtrack-plugin-32.dll
As the actual build folder is a GUID, it should be visible in the output log somewhere.

Though the 32-bit and 64-bit builds separately get tagged with the number of bits in the 
filename, we rename this to soundtrack-plugin.dll when installing to be consistent between
32-bit and 64-bit platforms.
