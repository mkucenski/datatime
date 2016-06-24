#AM_CXXFLAGS = -I../../ $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

bin_PROGRAMS = eventCorrelator
eventCorrelator_SOURCES = eventCorrelator.cpp
eventCorrelator_CXXFLAGS = $(BOOST_CPPFLAGS) $(POPT_CFLAGS) -I../../
eventCorrelator_LDADD = $(BOOST_DATE_TIME_LIB) $(POPT_LIBS) ../../libtimeUtils/src/libtimeUtils.a ../../libdelimText/src/libdelimText.a

