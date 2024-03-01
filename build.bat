@echo off
if not exist .\build mkdir .\build
pushd .\build
..\ctime\build\ctime -begin build_time.ctm

cl -DASSERT_ENABLE=1 -Zi -EHsc -GR- -Gm- -Od -Oi -MT ..\src\main.cpp gdi32.lib user32.lib winmm.lib opengl32.lib

..\ctime\build\ctime -end build_time.ctm
popd
