@echo off
cls

@set COMMON_COMPILER_FLAGS=/std:c++20 /Zi /WX /nologo /utf-8 /DTEMPEST_RELEASE -O2

pushd ..\build\

cl %COMMON_COMPILER_FLAGS% /MD /Fe"game_loader.exe" ../code/game_loader_windows.cpp /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib shell32.lib winmm.lib ole32.lib opengl32.lib

cl %COMMON_COMPILER_FLAGS% /LD /MD /Fe"game.dll" ../code/game.cpp /link user32.lib gdi32.lib shell32.lib

rmdir /s /q "..\release"
rmdir /s /q "..\release\tempest"
mkdir "..\release"
mkdir "..\release\tempest"

copy game.dll ..\release\tempest
copy game_loader.exe ..\release\tempest
move ..\release\tempest\game.dll ..\release\tempest\tempest.dll
move ..\release\tempest\game_loader.exe ..\release\tempest\tempest.exe
copy ..\attributions.txt ..\release\tempest
xcopy ..\data ..\release\tempest\data /E /I

del ..\release\tempest.zip
powershell Compress-Archive ..\release\tempest ..\release\tempest.zip

popd