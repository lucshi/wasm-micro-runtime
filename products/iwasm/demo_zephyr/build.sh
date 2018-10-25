emcc -g -O3 *.c -s WASM=1 -s SIDE_MODULE=1 -s TOTAL_MEMORY=65536 -s TOTAL_STACK=65536
cp -a a.out.wasm test.wasm
./jeffdump -o ../test_wasm.h -n wasm_test_file test.wasm
