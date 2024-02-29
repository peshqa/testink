@echo off
if not exist .\build mkdir .\build
pushd .\build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
popd
