@echo off

mkdir build\
pushd build\


call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\auxiliary\build\vcvarsall.bat" x64

cl -FC -nologo -Zi "C:\Program Files (x86)\projects\rasterizer\platform.c" user32.lib Gdi32.lib 

REM cl -FC -nologo -Zi "C:\Program Files (x86)\Projects\renderer\code\obj_loader.c" user32.lib Gdi32.lib 

popd
