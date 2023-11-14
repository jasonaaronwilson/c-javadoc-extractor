all: build

build:
	gcc -g -rdynamic -o javadoc-extractor-gcc *.c

format:
	clang-format -i *.c

clean:
	rm -rf *~ javadoc-extractor-gcc src-doc/

diff: clean
	git difftool HEAD

cfd: clean format diff
