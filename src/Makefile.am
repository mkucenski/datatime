AM_CXXFLAGS = -I../../../ $(BOOST_CPPFLAGS) $(POPT_CFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_DATE_TIME_LIB) $(POPT_LIBS)

bin_PROGRAMS = datatime
datatime_SOURCES = datatime.cpp
datatime_LDADD = ../../../libtimeUtils/build/src/libtimeUtils.a ../../../libdelimText/build/src/libdelimText.a

