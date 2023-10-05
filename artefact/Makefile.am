AUTOMAKE_OPTIONS = subdir-objects

if WITH_ARTEFACT
bin_PROGRAMS = artefact
endif

artefact_LDADD = $(LD_EFENCE) $(LUA_LIBS) $(PQ_LIBS) $(HTTPD_LIBS) \
	$(OPENSSL_LIBS) $(EXPAT_LIBS) $(PTHREAD_LIBS) \
	$(LEXLIB)

artefact_CXXFLAGS = -I../lib $(LUA_CFLAGS) $(PQ_CFLAGS) \
	$(HTTPD_CFLAGS) $(OPENSSL_CFLAGS) $(CONFIG_CXXFLAGS) \
	$(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)\
	$(CXXFLAGS_$(shell echo "$@" | sed -r 's/-/_/g'))\
	-I$(top_srcdir)/hugin -DWITH_HUG_SYSLOG -DWITH_HUG_FILTER 

artefact_CFLAGS = -DWITH_HUG_FILTER -DWITH_HUG_SYSLOG

if WITH_ARTEFACT
BUILT_SOURCES = \
	lex.yy.cc \
	y.tab.cc \
	y.tab.h
endif

PLIBS = \
	$(BUILT_SOURCES) \
	../lib/base64.cc \
	../lib/saxparser.cc \
	../lib/entry.cc \
	../lib/daemon.cc \
	../lib/exception.cc \
	../lib/httpd.cc \
	../lib/log.cc \
	../lib/mutex.cc \
	../lib/semaphore.cc \
	../lib/config.cc \
	../lib/tostring.cc \
	../lib/lua.cc \
	../lib/luautil.cc \
	../lib/tcpsocket.cc \
	../lib/mkdirr.cc \
	../lib/cpr.cc \
	../lib/utf8.cc \
	../lib/cprutils.cc \
	../lib/uid.cc \
	../lib/xml_encode_decode.cc

artefact_SOURCES = \
	$(PLIBS) \
	artefact.cc \
	connectionpool.cc \
	datawriter.cc \
	entryvalidator.cc \
	environment.cc \
	luadataclassifier.cc \
	luadatainterpreter.cc \
	luaexpat.cc \
	luapostgresql.cc \
	luatcpsocket.cc \
	postgresql.cc \
	queryhandler.cc \
	server.cc \
	transactionlogger.cc \
	translater_xml.cc \
	xmlparser.cc \
	$(top_srcdir)/hugin/hugin.c \
	$(top_srcdir)/hugin/hugin_filter.c \
	$(top_srcdir)/hugin/hugin_syslog.c

EXTRA_DIST = \
	connectionpool.h \
	datawriter.h \
	entryvalidator.h \
	environment.h \
	luadataclassifier.h \
	luadatainterpreter.h \
	luaexpat.h \
	luapostgresql.h \
	luatcpsocket.h \
	postgresql.h \
	queryhandler.h \
	server.h \
	transactionlogger.h \
	translater_xml.h \
	tree.h \
	xmlparser.h

if WITH_ARTEFACT
CXXFLAGS_artefact_lex.yy.o = -w

lex.yy.cc: ../lib/config.l
	$(LEX) -X ../lib/config.l
	mv lex.yy.c lex.yy.cc

y.tab.cc: ../lib/config.y
	$(YACC) ../lib/config.y
	mv y.tab.c y.tab.cc
	rm -f y.tab.h

y.tab.h: ../lib/config.y
	$(YACC) ../lib/config.y
#	mv y.tab.h ../lib/y.tab.h
	rm -f y.tab.c

#y.tab.h: ../lib/config.y
#	$(YACC) ../lib/config.y
#	rm -f y.tab.c

endif
