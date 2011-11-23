CC=gcc
LDFLAGS=-L/usr/local/BerkeleyDB/lib -L/usr/sfw/lib -ldb
CFLAGS=-O2 -I/usr/sfw/include -I/usr/local/BerkeleyDB/include -static -Wall $(CPPFLAGS)

dbgrep:
	$(CC) $(CFLAGS)  $< -o $@ $(LDFLAGS)

regex_t:
	$(CC) $(CFLAGS) $< -o $@ 
