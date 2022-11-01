all: servidor cliente

servidor: servidor.o
	gcc -o servidor servidor.c

cliente: cliente.o
	gcc -o cliente cliente.c

clean:
	rm cliente servidor *.o