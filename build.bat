@echo off
if not exist .\build mkdir .\build
pushd .\build

cl && (
    echo building...
) || (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)


cl -Zi ..\src\main.cpp

popd
