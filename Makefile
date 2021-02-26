all: clean ted

ted: src/*.c
	$(CC) -o ted src/*.c -lncursesw -Wall -W -pedantic -std=c99 -O2

dev: src/*.c
	rm ted -f
	$(CC) -o ted src/*.c -lncursesw -Wall -W -pedantic -std=c99 -fsanitize=address

clean:
	rm ted -f

install: src/*.c
	rm ted -f
	$(CC) -o ted src/*.c -lncursesw -std=c99 -O2
	cp ted ~/.local/bin
	mkdir -p ~/.config/ted/
	rm ~/.config/ted/docs/ -rf
	cp docs/ ~/.config/ted/ -r

