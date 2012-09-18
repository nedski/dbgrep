CC=gcc
LDFLAGS=-ldb
CFLAGS=-Wall $(CPPFLAGS)

dbgrep:
	$(CC) $(CFLAGS)  -o $@ dbgrep.c $(LDFLAGS)

dbgrep-dbg:
	$(CC) $(CFLAGS) -g -O1 -o dbgrep dbgrep.c $(LDFLAGS)

regex_t:
	$(CC) $(CFLAGS) -o $@ dbgrep.c
