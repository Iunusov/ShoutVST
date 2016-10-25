cd deps
call sync
cd ..

rd /s /q ShoutVST_DLL
rd /s /q build
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
ren ShoutVST_DLL\Release\ShoutVST.dll ShoutVST_x86.dll

rd /s /q build64
mkdir build64
cd build64
cmake -G"Visual Studio 14 2015 Win64" ..
cmake --build . --config Release
cd ..
ren ShoutVST_DLL\Release\ShoutVST.dll ShoutVST_x64.dll
PAUSE
