CXXFLAGS=-Wall -O2 -std=c++0x -DNDEBUG

all: proctempalert proctempview

proctempalert: proctempalert.cc proctemp.h
	g++ $(CXXFLAGS) -o $@ $@.cc -lsensors

proctempview: proctempview.cc *.h
	g++ $(CXXFLAGS) -o $@ $@.cc -lsensors -lncurses

clean:
	rm -f proctempalert proctempview

debug: CXXFLAGS=-Wall -std=c++0x -g
debug: clean all

install: clean all
	install --strip proctempalert /usr/local/bin
	install --strip proctempview /usr/local/bin
	install proctempalert.1 /usr/local/man/man1
	install proctempview.1 /usr/local/man/man1

uninstall:
	rm -f /usr/local/bin/proctempalert
	rm -f /usr/local/bin/proctempview
	rm -f /usr/local/man/man1/proctempalert.1
	rm -f /usr/local/man/man1/proctempview.1
