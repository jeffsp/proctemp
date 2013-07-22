bin_PROGRAMS = proctempalert proctempview
proctempalert_SOURCES = proctempalert.cc proctemp.h
proctempalert_LDADD = -lsensors
proctempview_SOURCES = proctempview.cc html.h options.h proctemp.h ui.h
proctempview_LDADD = -lsensors -lncurses

AM_CXXFLAGS = -std=c++0x
