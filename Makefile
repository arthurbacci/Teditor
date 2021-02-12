all: ted

ted: src/*.c
	$(CC) -o ted src/*.c -lncursesw -Wall -W -pedantic -std=c99 -O2

dev: src/*.c
	$(CC) -o ted src/*.c -lncursesw -Wall -W -pedantic -std=c99 -fsanitize=address

clean:
	rm ted -f

