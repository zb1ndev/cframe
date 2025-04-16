clear

gcc -c -o ./build/int/cframe.o ./src/cframe.c
ar rcs ./build/libcframe.a ./build/int/cframe.o

gcc -o ./build/simple-ssr ./examples/simple-ssr.c -L./build -l:libcframe.a

./build/simple-ssr