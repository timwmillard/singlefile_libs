CC = cc
CFLAGS = -Wall -Wextra

linalg.h: gen_linalg
	./gen_linalg > linalg.h

gen_linalg: gen_linalg.c
	$(CC) $(CFLAGS) -o gen_linalg gen_linalg.c

test: linalg_test
	./linalg_test

linalg_test: linalg_test.c linalg.h
	$(CC) $(CFLAGS) -o linalg_test linalg_test.c -lm

clean:
	rm -f gen_linalg linalg_test linalg.h
