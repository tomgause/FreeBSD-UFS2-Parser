fs-find: fs-find.c
	cc -g -Wall -pedantic -o fs-find fs-find.c

fs-cat: fs-cat.c
	cc -g -Wall -pedantic -o fs-cat fs-cat.c

.PHONY: clean
clean:
	rm -f fs-find fs-cat

