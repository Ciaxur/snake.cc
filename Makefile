CC = g++
3RD_PARTY_PATH= $(shell pwd)/3rd_party
FLAGS = -O3 -ggdb -std=c++20 -lfmt -Wall -Werror -l:libraylib.so
INCLUDES = -I"$(3RD_PARTY_PATH)/include" -I.
LINKER = -L"$(3RD_PARTY_PATH)/lib" -Wl,-rpath,"$(3RD_PARTY_PATH)/lib"

run: app
	./app

app: main.o
	$(CC) $(FLAGS) $(INCLUDES) $(LINKER) $^ -o app

main.o: ./src/main.cc
	$(CC) $(FLAGS) $(INCLUDES) ./src/main.cc -c
