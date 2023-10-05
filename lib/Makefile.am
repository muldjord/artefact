
BUILT_SOURCES = 
if DEPCONFIG
BUILT_SOURCES += lex.yy.cc y.tab.cc y.tab.h
endif

SOURCES = \
	$(BUILT_SOURCES) \
	base64.cc \
	cpr.cc \
	cprutils.cc \
	daemon.cc \
	delcom.cc \
	entry.cc \
	exception.cc \
	httpd.cc \
	log.cc \
	lua.cc \
	luautil.cc \
	md5.cc \
	mkdirr.cc \
	saxparser.cc \
	serial.cc \
	status.cc \
	tcpsocket.cc \
	tostring.cc \
	uid.cc \
	usb.cc \
	utf8.cc \
	semaphore.cc \
	mutex.cc \
	xml_encode_decode.cc \
	thread.cc

if DEPCONFIG
	SOURCES += config.cc 
endif

EXTRA_DIST = \
	base64.h \
	config.h \
	config.l \
	config.y \
	cpr.h \
	cprutils.h \
	daemon.h \
	delcom.h \
	entry.h \
	exception.h \
	httpd.h \
	iodevice.h \
	log.h \
	lua.h \
	luautil.h \
	md5.h \
	mkdirr.h \
	saxparser.h \
	serial.h \
	status.h \
	tcpsocket.h \
	tostring.h \
	uid.h \
	usb.h \
	utf8.h \
	semaphore.h \
	mutex.h \
	xml_encode_decode.h \
	thread.h

if DEPCONFIG

lex.yy.cc: config.l
	$(LEX) -X config.l
	mv lex.yy.c lex.yy.cc

y.tab.cc: config.y
	$(YACC) config.y
	mv y.tab.c y.tab.cc

y.tab.h: config.y
	$(YACC) config.y
	rm -f y.tab.c
endif
