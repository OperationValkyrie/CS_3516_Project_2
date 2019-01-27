CFLAGS = -c -g -Wall

all: p2

p2: project2.o student2.o
	gcc -g project2.c student2.c -o p2

project2.o: project2.c
	gcc $(CFLAGS) project2.c
	
student2.o: student2.c
	gcc $(CFLAGS) student2.c
	
clean:
	rm -f *.o p2