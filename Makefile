# build an executable named movies_by_year from main.c
all: main.c 
	gcc --std=gnu99 -o smallsh main.c

clean: 
	rm smallsh

test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./smallsh

leak-test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	valgrind --leak-check=yes ./smallsh

gdb-test:
	clear
	echo Run GDB with 'run'
	gcc -g --std=gnu99 -o smallsh main.c
	gdb ./smallsh
