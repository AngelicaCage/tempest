@echo off
cls

@set INCLUDES=/I..\code\glfw\include /I..\code\glad\include

rem Debug
@set COMMON_COMPILER_FLAGS=/std:c++20 /Zi /DEBUG:FULL /WX /nologo /utf-8

rem Release
rem @set COMMON_COMPILER_FLAGS=/std:c++20 /Zi /WX /nologo /utf-8

pushd ..\build\

cl %COMMON_COMPILER_FLAGS% /MDd /Fe"game_loader.exe" ../code/game_loader_windows.cpp /link /SUBSYSTEM:WINDOWS glfw/glfw3dll.lib user32.lib gdi32.lib shell32.lib winmm.lib

cl %COMMON_COMPILER_FLAGS% %INCLUDES% /LD /MDd /Fe"game.dll" ../code/game.cpp /link glfw/glfw3dll.lib user32.lib gdi32.lib shell32.lib

popd