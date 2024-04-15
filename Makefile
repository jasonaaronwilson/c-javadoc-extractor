all: build

install: build
	sudo install -m 755 c-markdown-extractor /usr/local/bin/

build:
	gcc -g -rdynamic -o c-markdown-extractor *.c

doc: build
	./c-markdown-extractor --output-dir=src-doc/ *.c

format:
	clang-format -i *.c

clean:
	rm -rf *~ c-markdown-extractor

diff: clean
	git difftool HEAD

cfd: clean format diff
