COMPILER = gcc
OBJECTS = main.o lexer.o build.o
FLAGS = -Wall -Wextra -Wpedantic
IGNORE = 

celda: $(OBJECTS)
	$(COMPILER) -o celda $(OBJECTS)

%.o: %.c
	$(COMPILER) -c $(FLAGS) $(IGNORE) $^

rm:
	rm -f $(OBJECTS) celda
