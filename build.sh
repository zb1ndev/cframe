clear

gcc -c -o ./build/int/cframe_unix.o ./src/cframe_unix.c
ar rcs ./build/libcframe.a ./build/int/cframe_unix.o

gcc -o ./build/simple-ssr ./examples/simple-ssr.c -L./build -l:libcframe.a

./build/simple-ssr