bin_PROGRAMS = proctempalert proctempview
proctempalert_SOURCES = proctempalert.cc proctemp.h
proctempalert_LDADD = -lsensors
proctempview_SOURCES = proctempview.cc html.h options.h proctemp.h ui.h
proctempview_LDADD = -lsensors -lncurses

man1_MANS = proctempalert.1 proctempview.1

AM_CXXFLAGS = -std=c++0x -Wall
