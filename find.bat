pushd .\src
set Wildcard=*.h *.cpp
findstr -s -n -i -l %1 %Wildcard%
popd
