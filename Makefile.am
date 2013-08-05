bin_PROGRAMS = thermalert therm
thermalert_SOURCES = thermalert.cc therm.h
thermalert_LDADD = -lsensors
therm_SOURCES = therm.cc html.h options.h therm.h ui.h
therm_LDADD = -lsensors -lncurses

man1_MANS = thermalert.1 therm.1

AM_CXXFLAGS = -std=c++0x -Wall
