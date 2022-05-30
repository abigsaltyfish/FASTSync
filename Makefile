cc = gcc
link = -lcrypto -lz
deps = String.h fileinfo.h fastcdc.h chunkList.h io.h hashmap.h match.h uthash.h socket.h
serverobj = fileinfo.o String.o chunkList.o fastcdc.o io.o hashmap.o socket.o server.o
clientobj = fileinfo.o String.o chunkList.o fastcdc.o io.o hashmap.o match.o socket.o client.o 

test:
	$(cc) -g fastcdc.c test.c -lcrypto -lz -o a.out

client: $(clientobj)
	$(cc) -o client.run $(clientobj) $(link)
	
server: $(serverobj)
	$(cc) -o server.run $(serverobj) $(link)

%.o: %.c $(deps)
	$(cc) -c -g $< -o $@

clean:
	rm -rf *.o

