@echo off
cls

if not exist .\build\int mkdir .\build\int
if not exist .\build mkdir .\build

gcc -c -o .\build\int\cframe.o .\src\cframe.c -lws2_32
gcc -c -o .\build\int\http.o .\src\http.c -lws2_32
gcc -c -o .\build\int\handler.o .\src\handler.c -lws2_32
gcc -c -o .\build\int\node.o .\src\node.c -lws2_32

ar rcs .\build\libcframe.a .\build\int\cframe.o .\build\int\http.o .\build\int\handler.o .\build\int\node.o
gcc -o .\build\simple-ssr.exe .\examples\simple-ssr.c -L.\build -l:libcframe.a -lws2_32

@echo on
.\build\simple-ssr.exe