mkdir -p obj/ bin/
nasm -felf64 -o obj/memcpy.s.o src/memcpy.s
g++ -std=c++20 -march=native -c -O3 -o obj/memcpy.cpp.o src/memcpy.cpp
g++ -o bin/memcpy obj/memcpy.*.o
