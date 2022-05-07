all: servidor.o
	gcc -pthread -lrt -o servidor servidor.o ../librerias/netutil.o ../librerias/split.o ../librerias/ioutil.o

servidor.o: servidor.c
	gcc -c -I../librerias -o servidor.o servidor.c

clean:
	rm -f servidor  *.o