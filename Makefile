CC = clang
CFLAGS = -O2
SOURCE = src/*.c
INCLUDE = -I./include
OUT = -o
EXECUTABLE = rdfgen

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(OUT) $(EXECUTABLE) $(SOURCE)

install:
	cp $(EXECUTABLE) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
