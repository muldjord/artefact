AUTOMAKE_OPTIONS = subdir-objects

if WITH_BARCODE
bin_PROGRAMS = barcodeextract
endif

barcodeextract_LDADD = ../libartefact/libartefact.la $(PQ_LIBS) 
barcodeextract_CXXFLAGS = -I$(top_srcdir)/libartefact $(PQ_CFLAGS) \
	-DWITH_HUG_SYSLOG -I..//hugin

barcodeextract_CFLAGS = -DWITH_HUG_SYSLOG

HUGIN = \
	..//hugin/hugin.c \
	..//hugin/hugin_syslog.c

barcodeextract_SOURCES = \
	$(HUGIN) \
	main.cc \
	pgwork.cc 

EXTRA_DIST = \
	pgwork.h 
