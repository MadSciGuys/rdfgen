# Travis Whitaker 2013
# twhitak@its.jnj.com

CC = clang
CFLAGS =
INCLUDE = -I./include
EXECUTABLE = rdfgen

all: rdfgen.o fileIO.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXECUTABLE) src/main.c rdfgen.o fileIO.o

rdfgen.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen.c

fileIO.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/fileIO.c

install:
	cp $(EXECUTABLE) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
