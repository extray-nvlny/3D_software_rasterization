#echo off

mkdir ..\build\
pushd ..\build\


call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\auxiliary\build\vcvarsall.bat" x64

cl -nologo -Zi "C:\Program Files (x86)\Projects\renderer\code\platform.c" user32.lib Gdi32.lib 

popd