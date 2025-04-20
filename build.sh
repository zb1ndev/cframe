clear

gcc -c -o ./build/int/cframe.o ./src/cframe.c
gcc -c -o ./build/int/http.o ./src/http-server/http.c
gcc -c -o ./build/int/handler.o ./src/http-request-handler/handler.c

ar rcs ./build/libcframe.a ./build/int/cframe.o ./build/int/http.o ./build/int/handler.o
gcc -o ./build/simple-ssr ./examples/simple-ssr.c -L./build -l:libcframe.a

./build/simple-ssr