COMPILER = gcc
OBJECTS = main.o lexer.o build.o arith.o
FLAGS = -Wall -Wextra -Wpedantic
IGNORE = -Wno-switch

celda: $(OBJECTS)
	$(COMPILER) -o celda $(OBJECTS) -lm
	./celda normal/expressions

%.o: %.c
	$(COMPILER) -c $(FLAGS) $(IGNORE) $^

rm:
	rm -f $(OBJECTS) celda
