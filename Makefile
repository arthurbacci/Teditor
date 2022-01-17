# Default install location
PREFIX=/usr/local

# The user owns CPPFLAGS, CFLAGS, LDFLAGS, and friends. Use our own internal
# variables to include our flags. Our flags are listed first so the user
# can always override our default choices.
TED_DEBUG=-Wall -O0 -g3 -pedantic -std=c99
TED_ANALYZE=-Wall -O1 -g3 -pedantic -std=c99
TED_RELEASE=-Wall -O3 -g2 -pedantic -std=c99
TED_LIBS=-lncursesw

all: ted

ted: src/*.c
	$(CC) -o ted $^ $(TED_RELEASE) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TED_LIBS) $(LIBS)

debug:  src/*.c
	$(CC) -o ted $^ $(TED_DEBUG) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TED_LIBS) $(LIBS)

asan: src/*.c
	$(CC) -o ted $^ $(TED_ANALYZE) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TED_LIBS) $(LIBS) -fsanitize=address

ubsan: src/*.c
	$(CC) -o ted $^ $(TED_ANALYZE) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TED_LIBS) $(LIBS) -fsanitize=undefined

clean:
	rm -f ted

distclean:
	rm -rf *.o ted

install: src/*.c
	rm -f ted
	$(CC) -o ted $^ $(TED_RELEASE) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(TED_LIBS) $(LIBS)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp ted $(DESTDIR)$(PREFIX)/bin/
	chmod 0755 $(DESTDIR)$(PREFIX)/bin/ted
	mkdir -p ~/.config/ted/
	rm -rf ~/.config/ted/docs/
	cp -r docs/ ~/.config/ted/

