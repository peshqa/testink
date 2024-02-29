@echo off
if not exist .\build mkdir .\build
pushd .\build
..\ctime\build\ctime -stats build_time.ctm
popd
