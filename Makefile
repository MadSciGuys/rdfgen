# Travis Whitaker 2013
# twhitak@its.jnj.com

CC = clang
CFLAGS = -O2
INCLUDE = -I./include
EXECUTABLE = rdfgen

all: rdfgen.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXECUTABLE) src/main.c rdfgen.o

rdfgen.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen.c

install:
	cp $(EXECUTABLE) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
