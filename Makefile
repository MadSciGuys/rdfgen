# Travis Whitaker 2013
# twhitak@its.jnj.com

CC = clang
CFLAGS = -Wall -Wextra -pedantic -O2
INCLUDE = -I./include
EXECUTABLE = rdfgen
LINKARGS = -lrt

all: interface.o parser.o generator.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $(EXECUTABLE) src/main.c interface.o parser.o generator.o $(LINKARGS)

interface.o: parser.o
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/interface.c

parser.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/parser.c

generator.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/generator.c

install:
	cp $(EXECUTABLE) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
	rm -f debug-rdfgen

debug: interface.o.debug parser.o.debug generator.o.debug
	$(CC) -O0 -g -Wall -Wextra -pedantic $(INCLUDE) -o debug-rdfgen src/main.c interface.o parser.o generator.o $(LINKARGS)

interface.o.debug: parser.o.debug
	$(CC) -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/interface.c

parser.o.debug:
	$(CC) -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/parser.c

generator.o.debug:
	$(CC) -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/generator.c
