COMPILER = gcc
OBJECTS = main.o lexer.o build.o arith.o
FLAGS = -Wall -Wextra -Wpedantic
IGNORE = -Wno-switch

celda: $(OBJECTS)
	$(COMPILER) -o celda $(OBJECTS) -lm -lpthread
	./celda normal/types 2> error > output
	cat output

%.o: %.c
	$(COMPILER) -c $(FLAGS) $(IGNORE) $^

rm:
	rm -f $(OBJECTS) celda && clear
