@echo off
cls

@set COMMON_COMPILER_FLAGS=/std:c++20 /Zi /DEBUG:FULL /WX /nologo /utf-8
@set INCLUDES=/I..\code\glfw\include /I..\code\glad\include

pushd ..\build\

cl %COMMON_COMPILER_FLAGS% /MDd /Fe"game_loader.exe" ../code/game_loader_windows.cpp /link /SUBSYSTEM:WINDOWS glfw/glfw3.lib user32.lib gdi32.lib shell32.lib

cl %COMMON_COMPILER_FLAGS% %INCLUDES% /LD /MDd /Fe"game.dll" ../code/game.cpp /link glfw/glfw3.lib user32.lib gdi32.lib shell32.lib

popd