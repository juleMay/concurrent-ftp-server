all: cliente.o
	gcc -o cliente cliente.o ../librerias/netutil.o ../librerias/split.o ../librerias/ioutil.o

cliente.o: cliente.c
	gcc -c -I../librerias -o cliente.o cliente.c

clean:
	rm -f cliente *.o