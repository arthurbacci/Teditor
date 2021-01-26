all: ted

ted: ted.c
	$(CC) -o ted ted.c -lncursesw -Wall -W -pedantic -std=c99

dev: ted.c
	$(CC) -o ted ted.c -lncursesw -Wall -W -pedantic -std=c99 -fsanitize=address

clean:
	rm ted
