COMPILER = gcc
OBJECTS = main.o lexer.o build.o arith.o
FLAGS = -Wall -Wextra -Wpedantic
IGNORE = -Wno-switch -Wno-unused-parameter

celda: $(OBJECTS)
	$(COMPILER) -o celda $(OBJECTS) -lm -lpthread
	./celda normal/expressions #2> error > output

%.o: %.c
	$(COMPILER) -c $(FLAGS) $(IGNORE) $^

rm:
	rm -f $(OBJECTS) celda && clear
