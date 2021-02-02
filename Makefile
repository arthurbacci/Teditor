all: ted

ted: src/*
	$(CC) -o ted src/* -lncursesw -Wall -W -pedantic -std=c99

dev: src/*
	$(CC) -o ted src/* -lncursesw -Wall -W -pedantic -std=c99 -fsanitize=address

clean:
	rm ted

