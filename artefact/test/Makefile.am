AUTOMAKE_OPTIONS = subdir-objects
TESTS = postgresql mkdirr
# pseudoclass

check_PROGRAMS = $(TESTS)

postgresql_CXXFLAGS = $(CPPUNIT_CFLAGS) -DOUTPUT=\"postgresql\" \
  -I.. -I../../lib $(PQ_CFLAGS) -I$(top_srcdir)/hugin
postgresql_LDFLAGS = $(CPPUNIT_LIBS) $(PQ_LIBS)
postgresql_SOURCES = test.cc test_postgresql.cc $(top_srcdir)/hugin/hugin.c

mkdirr_CXXFLAGS = $(CPPUNIT_CFLAGS) -DOUTPUT=\"mkdirr\" -I../../lib \
  -I$(top_srcdir)/hugin
mkdirr_LDFLAGS = $(CPPUNIT_LIBS)
mkdirr_SOURCES = test.cc test_mkdirr.cc $(top_srcdir)/hugin/hugin.c







#PKG_EXPAT=$(pkg-config --list-all | grep expat)
#EXPAT_LIBS=`if [ -z "${PKG_EXPAT}" ]; then echo "-lexpat"; else pkg-config --libs expat; fi`
#EXPAT_CFLAGS=`if [ -z "${PKG_EXPAT}" ]; then echo ""; else pkg-config --cflags expat; fi`
#
#CFLAGS=\
#  `cppunit-config --cflags` -I../../ -I../../lib \
#  `pkg-config libpqxx --cflags` \
#  `pkg-config lua --cflags` \
#  ${EXPAT_CFLAGS}
#
#LIBS = \
#  `cppunit-config --libs` \
#  `pkg-config libpqxx --libs` -lpq \
#  `pkg-config lua --libs` \
#  ${EXPAT_LIBS}
#
#all: postgresql pseudoclass mkdirr
#
#postgresql:
#	g++ ${CFLAGS} -DOUTPUT=\"$@\" test.cc test_$@.cc ${LIBS} -o test_$@
#	./test_$@
#
#pseudoclass:
#	g++ ${CFLAGS} -DOUTPUT=\"$@\" test.cc test_$@.cc ${LIBS} -o test_$@
#	./test_$@
#
#mkdirr:
#	g++ ${CFLAGS} -DOUTPUT=\"$@\" test.cc test_$@.cc ${LIBS} -o test_$@
#	./test_$@
