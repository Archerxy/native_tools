# mingw build lib
1. gcc -c reentrant_lock.c list.c log.c logger.c map.c json.c -fPIC -static-libgcc -static-libstdc++ --std=c99 -O3 -falign-functions  -falign-jumps -falign-labels  -falign-loops -finline-small-functions -lpthread
2. ar -x libpthread.a
3. ar -csr libarcher_tool-win64.a *.o

# mingw build dynamic lib
gcc -fPIC -shared reentrant_lock.c list.c log.c logger.c map.c json.c -static-libgcc -static-libstdc++ --std=c99 -O3 -falign-functions  -falign-jumps -falign-labels  -falign-loops -finline-small-functions -lpthread -o libarcher_tool.dll







