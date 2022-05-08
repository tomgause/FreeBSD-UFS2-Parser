fs-find: fs-find.c
	cc -g -Wall -pedantic -o fs-find fs-find.c

.PHONY: clean
clean:
	rm -f fs-find
