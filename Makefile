# Travis Whitaker 2012
# twhitak@its.jnj.com

CC = clang
CFLAGS =
INCLUDE = -I./include
EXECUTABLE = rdfgen

all: rdfgen.o fileIO.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXECUTABLE) src/main.c rdfgen.o fileIO.o

rdfgen.o:
	$(CC) -c $(INCLUDE) src/rdfgen.c

fileIO.o:
	$(CC) -c $(INCLUDE) src/fileIO.c

install:
	cp $(EXECUTABLE) /usr/bin/local/

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
