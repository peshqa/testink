@echo off
if not exist .\build mkdir .\build
pushd .\build
cl -DASSERT_ENABLE=1 -Zi -EHsc ..\src\main.cpp gdi32.lib user32.lib
popd
