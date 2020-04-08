SRC=src/
BLD=build/
TST=tests/

FLAGS= -Wall

.PHONY: all build tests clean

all: build

build: tracker

tests: $(BLD)utils.o $(BLD)thpool.o $(BLD)hash_table.o
	cc $(FLAGS) -o $(BLD)$@ $(TST)test_hash_table.c $^ -pthread

clean:
	rm -rf $(SRC)*.o *~ \#*#
	rm -rf $(BLD)*

tracker: $(BLD)utils.o $(BLD)thpool.o $(BLD)hash_table.o
	cc $(FLAGS) -o $(BLD)$@ $(SRC)$@.c $^ -pthread

$(BLD)utils.o:
		cc $(FLAGS) -o $@ -c $(SRC)utils.c

$(BLD)thpool.o:
		cc $(FLAGS) -o $@ -c $(SRC)thpool.c -D THPOOL_DEBUG -pthread

$(BLD)hash_table.o:
		cc $(FLAGS) -o $@ -c $(SRC)hash_table.c
