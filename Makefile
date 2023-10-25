all: build

build:
	gcc -o javadoc-extractor-gcc *.c

format:
	clang-format -i *.c

clean:
	rm -rf *~ javadoc-extractor-gcc

diff: clean
	git difftool HEAD
