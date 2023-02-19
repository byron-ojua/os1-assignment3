# build an executable named movies_by_year from main.c
all: main.c 
	gcc --std=gnu99 -o smallsh main.c

clean: 
	rm smallsh

test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./smallsh

testscript-basic:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./basicp3testscript 2>&1

testscript-1:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./p3testscript 2>&1

testscript-2:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./p3testscript 2>&1 | more

testscript-3:
	clear
	gcc --std=gnu99 -o smallsh main.c
	./p3testscript > mytestresults 2>&1 

leak-test:
	clear
	gcc --std=gnu99 -o smallsh main.c
	valgrind --leak-check=yes ./smallsh

gdb-test:
	clear
	echo Run GDB with 'run'
	gcc -g --std=gnu99 -o smallsh main.c
	gdb ./smallsh
