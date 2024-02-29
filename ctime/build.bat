@echo off
if not exist .\build mkdir .\build
pushd .\build
cl -Zi -EHsc -GR- -Gm- -O2 -Oi -MT ..\ctime.c winmm.lib
popd
