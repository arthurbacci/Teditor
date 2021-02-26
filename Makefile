all: ted

CFLAGS=-Wall -W -pedantic -std=c99 -O2
LIBS=-lncursesw

PREFIX=/usr/local

ted: src/*.c
	$(CC) -o ted $^ $(CFLAGS) $(LIBS)

asan: src/*.c
	$(CC) -o ted $^ $(CFLAGS) $(LIBS) -fsanitize=address

clean:
	rm ted -f

install: src/*.c
	rm ted -f
	$(CC) -o ted $^ $(CFLAGS) $(LIBS)
	cp ted $(PREFIX)/bin/
	mkdir -p ~/.config/ted/
	rm ~/.config/ted/docs/ -rf
	cp docs/ ~/.config/ted/ -r

