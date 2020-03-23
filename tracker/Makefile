SRC=src/
BLD=build/
TST=tst/

FLAGS= -Wall -std=c99

.PHONY: all build tests clean

all: build

build: tracker

tests:

clean:
	rm -rf $(SRC)*.o *~ \#*#
	rm -rf $(BLD)*

tracker: $(BLD)utils.o $(BLD)thpool.o 
	cc $(FLAGS) -o $@ $(SRC)$@.c $^ 

$(BLD)utils.o:
		cc $(FLAGS) -o $@ -c $(SRC)utils.c 

$(BLD)thpool.o:
		cc $(FLAGS) -o $@ -c $(SRC)thpool.c -D THPOOL_DEBUG -pthread
