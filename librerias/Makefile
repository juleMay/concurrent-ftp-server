all: netutil.o split.o ioutil.o

netutil.o: netutil.c
	gcc -c -o netutil.o netutil.c

split.o: split.c
	gcc -c -o split.o split.c

ioutil.o: ioutil.c
	gcc -c -o ioutil.o ioutil.c

clean:
	rm -f *.o