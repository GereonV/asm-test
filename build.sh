mkdir -p obj/ bin/
nasm -felf64 -o obj/clamp.s.o src/clamp.s
g++ -std=c++20 -c -O3 -o obj/clamp.cpp.o src/clamp.cpp
g++ -o bin/clamp obj/clamp.*.o
