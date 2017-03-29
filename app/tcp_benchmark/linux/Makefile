all: sender receiver

sender: sender.o common.o
	gcc sender.o common.o -o sender

receiver: receiver.o common.o
	gcc receiver.o common.o -o receiver

sender.o: sender.c
	gcc -ggdb3 -c sender.c -o sender.o

receiver.o: receiver.c
	gcc -ggdb3 -c receiver.c -o receiver.o

common.o: ../common.c
	gcc -ggdb3 -c ../common.c -o common.o

clean:
	rm -f *.o sender receiver
