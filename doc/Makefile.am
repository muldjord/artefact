SUBDIRS =

docdir = ${prefix}/share/doc/artefact-@VERSION@

EXTRA_DIST = \
	doxygen.cfg.in

all:

doc: doxygen.cfg
#	echo "doc: " && pwd &&  echo "distdir: " && echo $(distdir)
#	rm -rf html/ refman.pdf
	$(DOXYGEN) doxygen.cfg
#       $(MAKE) -C latex/
#       mv latex/refman.pdf ./refman.pdf

dist-hook: doxygen.cfg
	echo "dist-hook: " && pwd
	cd $(srcdir) && pwd && rm -rf html refman.pdf && $(DOXYGEN) doxygen.cfg
	cp -rp html ${distdir}

clean-local:
	echo "clean-local: " && pwd
	rm -rf latex/
	rm -rf html/
	rm -f *~
	rm -f doxygen.log
	rm -f doxygen.cfg

maintainer-clean-local: clean-local
	echo "maintainer-clean-local: " && pwd
	rm -rf html refman.pdf

install-data-hook: doc
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	mkdir -p html
	cp -rp html $(DESTDIR)$(docdir)

uninstall-hook:
	chmod +w -R $(DESTDIR)${docdir}/html
	rm -rf $(DESTDIR)${docdir}/html

## We borrow guile's convention and use @-...-@ as the substitution
## brackets here, instead of the usual @...@.  This prevents autoconf
## from substituting the values directly into the left-hand sides of
## the sed substitutions.
doxygen.cfg: doxygen.cfg.in Makefile
	sed < $< > $@.tmp -e 's:@-top_srcdir-@:${top_srcdir}:g'	\
	-e 's:@-name-@:${PACKAGE}:g' \
	-e 's:@-version-@:${VERSION}:g' \
	-e 's:@-enable_dot-@:${enable_dot}:g' \
	-e 's:@-enable_html_docs-@:${enable_html_docs}:g' \
	-e 's:@-enable_latex_docs-@:${enable_latex_docs}:g'
	mv $@.tmp $@

