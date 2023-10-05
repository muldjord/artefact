AUTOMAKE_OPTIONS = subdir-objects

if WITH_LIBARTEFACT
lib_LTLIBRARIES = libartefact.la
endif

libartefact_la_LIBADD = $(CURL_LIBS) $(EXPAT_LIBS)

#libartefact_la_LDFLAGS = -export-dynamic -no-undefined -version-info 42:0:0

libartefact_la_CXXFLAGS = -I../lib $(CURL_CFLAGS) $(EXPAT_CFLAGS)

libartefact_la_SOURCES = \
	libartefact.cc \
	responsparser.cc \
	resultparser.cc \
	../lib/base64.cc \
	../lib/saxparser.cc

if WITH_LIBARTEFACT
include_HEADERS = \
	libartefact.h \
	../lib/libartefactstatus.h
endif

EXTRA_DIST = \
	libartefact.h \
	libartefact_private.h \
	responsparser.h	\
	resultparser.h \
	../lib/base64.h \
	../lib/libartefactstatus.h \
	../lib/saxparser.h
