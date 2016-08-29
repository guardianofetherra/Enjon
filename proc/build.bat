@echo off
pushd bin
cl /w -Zi /MP /Femain.exe ^
..\src\*.cpp ..\src\Graphics\*.cpp ..\src\IO\*.cpp ..\src\Math\*.cpp  ..\src\Physics\*.cpp ..\src\Utils\*.cpp  ..\src\Editor\*.cpp ..\src\AI\*.cpp ..\src\ECS\*.cpp ..\src\BehaviorTree\*.cpp ^
..\IsoARPG\src\*.cpp ^
/I ..\include /I ..\deps\include /I ..\IsoARPG\include ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /LIBPATH:"..\deps\lib\" Opengl32.lib SDL2.lib SDL2main.lib glew32.lib glew32s.lib freetype.lib
popd
