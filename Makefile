CC=gcc
LDFLAGS=-ldb
CFLAGS=-Wall $(CPPFLAGS)

dbgrep:
	$(CC) $(CFLAGS)  -o $@ $(LDFLAGS) dbgrep.c

regex_t:
	$(CC) $(CFLAGS) -o $@ dbgrep.c
