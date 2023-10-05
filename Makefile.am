AUTOMAKE_OPTIONS = gnu subdir-objects

SUBDIRS = lib artefact tools etc lib man gentoo/init.d lua \
	libartefact barcodeextract oftdevproxy 

DISTDIRS = lib artefact tools etc man lua \
	libartefact oftdevproxy barcodeextract

EXTRA_DIST = \
	libartefact.pc.in \
	hugin/hugin.h \
	hugin/hugin.hpp \
	hugin/hugin.c \
	hugin/hugin_util.h \
	hugin/hugin_filter.h \
	hugin/hugin_syslog.h \
	hugin/hugin_syslog.c 

ACLOCAL_AMFLAGS = -I m4

if WITH_LIBARTEFACT
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	libartefact.pc 
endif

.PHONY: doc
doc:
	$(MAKE) -C doc doc

test:
	$(MAKE) -C lib test
	$(MAKE) -C artefact test
	$(MAKE) -C libartefact test
	$(MAKE) -C pidiod test
	$(MAKE) -C libpidio test
	$(MAKE) -C mcr test

release:
	cvs2cl #-FSF -P --hide-filenames
	$(MAKE) dist

rpm:
	@echo "## Creating rpms"
	@echo "# Delete old rpmbuild (if one exists):"
	rm -rf rpmbuild

#	@echo "# Configuring package with artefact, libartefact and oftdevproxy:"
#	make distclean
#	./configure --with-artefact --with-libartefact --with-oftdevproxy

	@echo "# Create rpmbuild directories:"
	mkdir -p rpmbuild/{RPMS,BUILD,SOURCES}
	make dist
	cp artefact-$(VERSION).tar.gz rpmbuild/SOURCES

	rpmbuild -ba artefact-lua.spec --define="%_topdir ${PWD}/rpmbuild"
	rpmbuild -ba artefact.spec --define="%_topdir ${PWD}/rpmbuild"
	rpmbuild -ba libartefact.spec --define="%_topdir ${PWD}/rpmbuild"
	rpmbuild -ba oftdevproxy.spec --define="%_topdir ${PWD}/rpmbuild"

	@echo " # Moving rpms:"
	mv rpmbuild/RPMS/*/*.rpm .
