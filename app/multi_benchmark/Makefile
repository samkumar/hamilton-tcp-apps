all: multireceiver

multireceiver: multireceiver.o
	gcc multireceiver.o -pthread -lrt -o multireceiver

multireceiver.o: multireceiver.c
	gcc -ggdb3 -pthread -c multireceiver.c -o multireceiver.o

clean:
	rm -f multireceiver *.o
