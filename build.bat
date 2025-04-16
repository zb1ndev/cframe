@echo off
cls

if not exist .\build\int mkdir .\build\int
if not exist .\build mkdir .\build

gcc -c -o .\build\int\cframe.o .\src\cframe.c -lws2_32
ar rcs .\build\libcframe.a .\build\int\cframe.o

gcc -o .\build\simple-ssr.exe .\examples\simple-ssr.c -L.\build -l:libcframe.a -lws2_32

@echo on
.\build\simple-ssr.exe