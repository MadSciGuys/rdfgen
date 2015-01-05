# Travis Whitaker 2013-2015
# twhitak@its.jnj.com

CC = clang
CFLAGS = -Wall -Wextra -pedantic -O2 -pipe -march=native
INCLUDE = -I./include
EXECUTABLE = rdfgen
LINKARGS =
COLOR = yes
TIME = yes
INSTALLDIR = /usr/local/bin/

ifeq ($(COLOR),no)
CFLAGS += -DNO_COLOR
endif

ifeq ($(TIME),no)
CFLAGS += -DNO_TIME
else
LINKARGS += -lrt
endif

all: interface.o parser.o generator.o
	$(CC) $(CFLAGS) $(INCLUDE)  -o $(EXECUTABLE) src/main.c interface.o parser.o generator.o $(LINKARGS)

interface.o: parser.o
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/interface.c

parser.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/parser.c

generator.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) src/rdfgen/generator.c

install:
	cp $(EXECUTABLE) $(INSTALLDIR)

uninstall:
	rm -f $(INSTALLDIR)$(EXECUTABLE)

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
	rm -f debug-rdfgen

debug: interface.o.debug parser.o.debug generator.o.debug
	$(CC) -pipe -O0 -g -Wall -Wextra -pedantic $(INCLUDE) -o debug-rdfgen src/main.c interface.o parser.o generator.o $(LINKARGS)

interface.o.debug: parser.o.debug
	$(CC) -pipe -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/interface.c

parser.o.debug:
	$(CC) -pipe -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/parser.c

generator.o.debug:
	$(CC) -pipe -c -O0 -g -Wall -Wextra -pedantic $(INCLUDE) src/rdfgen/generator.c
