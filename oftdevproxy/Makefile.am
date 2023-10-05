AUTOMAKE_OPTIONS = subdir-objects

if WITH_OFTDEVPROXY
bin_PROGRAMS = oftdevproxy
endif

oftdevproxy_LDADD = -lexpat ../libartefact/libartefact.la $(OPENSSL_LIBS)
#oftdevproxy_LDADD = -lexpat -lartefact $(OPENSSL_LIBS)
oftdevproxy_CXXFLAGS = -I../lib -I../libartefact $(OPENSSL_CFLAGS) \
	-DWITH_HUG_SYSLOG -I../hugin

oftdevproxy_CFLAGS = -DWITH_HUG_SYSLOG

HUGIN = \
	../hugin/hugin.c \
	../hugin/hugin_syslog.c

PLIBS = \
	../lib/saxparser.cc \
	../lib/exception.cc \
	../lib/log.cc

oftdevproxy_SOURCES = \
	$(PLIBS) \
	$(HUGIN) \
	oftdevproxy.cc \
	requestparser.cc \
	artefact.cc

EXTRA_DIST = \
	request.h \
	requestparser.h \
	artefact.h
