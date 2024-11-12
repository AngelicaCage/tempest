@echo off
cls

@set common_compiler_flags=/std:c++20 /Zi /DEBUG:FULL /WX /nologo /utf-8

pushd ..\build\

cl %common_compiler_flags% /Fe"game_loader.exe" ../code/game_loader_windows.cpp

cl %common_compiler_flags% /LD /Fe"game.dll" ../code/game.cpp

popd