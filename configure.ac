AC_INIT([artefact], [0.2.1])
AC_CONFIG_SRCDIR([artefact/artefact.cc])
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AM_PROG_CC_C_O

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AM_CONFIG_HEADER(sysconfig.h)
AC_STDC_HEADERS

dnl ======================
dnl Init pkg-config
dnl ======================
PKG_PROG_PKG_CONFIG(0.23)

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(lib/Makefile)
AC_CONFIG_FILES(tools/Makefile)
AC_CONFIG_FILES(etc/Makefile)
AC_CONFIG_FILES(man/Makefile)
dnl AC_CONFIG_FILES(doc/Makefile)
dnl AC_CONFIG_FILES(lua/Makefile)
dnl AC_CONFIG_FILES(xml/Makefile)
AC_CONFIG_FILES(gentoo/init.d/Makefile)

####################
# Dependency list
DEP_OPENSSL=no
DEP_POSTGRES=no
DEP_MICROHTTPD=no
DEP_LUA=no
DEP_CONFIG=no
DEP_PTHREAD=no
DEP_EXPAT=no
DEP_USB=no
DEP_CURL=no
DEP_MYSQL=no
DEP_UUID=no
DEP_LIBARTEFACT=no

if test x$with_artefact$with_oftdevproxy$with_libartefact$with_lua$with_barcode == x; then
  AC_MSG_WARN([*******************************************************]);
  AC_MSG_WARN([*** Nothing has been set to be build - building all ***]);
  AC_MSG_WARN([*** ( Use the --with-XXXX commands to build only    ***]);
  AC_MSG_WARN([***   selected components )                         ***]);
  AC_MSG_WARN([*******************************************************]);
  with_artefact=yes
  with_oftdevproxy=yes
  with_libartefact=yes
  with_lua=yes
fi

################################
# Create the ETC var i config.h
if echo "$prefix" | grep "NONE" > /dev/null
then
   MYPREFIX="/usr/local"
else
   MYPREFIX="${prefix}"
fi
AC_SUBST(MYPREFIX)
AC_DEFINE_UNQUOTED(ETC, "$MYPREFIX/etc", [The path to the config files])

##################################
# Check if tests should be built.
AC_ARG_WITH(test,
        [  --with-test             build tests (default=no)],
        [],
        [with_test=no])
if test x$with_test == xyes; then
  PKG_CHECK_MODULES(CPPUNIT, cppunit >= 1.9.6)
#  AM_PATH_CPPUNIT(1.9.6)
  AC_CONFIG_FILES(artefact/test/Makefile)
#  AC_CONFIG_FILES(lua/Makefile)
fi

####################
# Setup debug flags
AC_ARG_WITH(debug,  [  --with-debug            build with debug support])
if test x$with_debug == xyes; then
  AC_MSG_WARN([*** Building with debug support!])
  CXXFLAGS="$CXXFLAGS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fstack-protector -Wall -Werror -g"
  AC_DEFINE_UNQUOTED(WITH_DEBUG, ,
    [The project is configured to use debug output])
fi

###############
# Setup efence
AC_ARG_ENABLE(efence,
  [  --enable-efence        enable efence - for debugging only (no)],
  [], [ enable_efence=no])
if test "x$enable_efence" = xno; then
  enable_efence=no
else
  LD_EFENCE="-lefence"
  AC_SUBST(LD_EFENCE)
  AC_DEFINE_UNQUOTED(USE_EFENCE, , [Use the efence includes])
fi

#######################################
#
#          Component setup
#
#######################################

##########################
# Artefact server support
AC_ARG_WITH(artefact,
	[  --with-artefact         build with artefact server support (default=no)],	[],
	[with_server=no])
if test x$with_artefact == xyes; then
  DEP_OPENSSL=yes
  DEP_POSTGRES=yes
  DEP_MICROHTTPD=yes
  DEP_LUA=yes
  DEP_EXPAT=yes
  DEP_CONFIG=yes
  DEP_PTHREAD=yes


  ARTEFACT_INIT=gentoo/init.d/artefact
  AC_SUBST(ARTEFACT_INIT)

  AC_CONFIG_FILES(gentoo/init.d/artefact)
  AC_CONFIG_FILES(etc/artefact.conf.example)
  AC_CONFIG_FILES(etc/artefact)
fi
AC_CONFIG_FILES(artefact/Makefile)

######################
# OftDevProxy support
AC_ARG_WITH(oftdevproxy,
	[  --with-oftdevproxy      build with oftdevproxy support (default=no)],
	[], [with_oftdevproxy=no])
if test x$with_oftdevproxy == xyes; then
  DEP_EXPAT=yes
  DEP_CONFIG=yes
#  DEP_LIBARTEFACT=yes

  AC_CONFIG_FILES(etc/oftdevproxy)
  # Force libartefact
  with_libartefact=yes
fi
AC_CONFIG_FILES(oftdevproxy/Makefile)

######################
# libartefact support
AC_ARG_WITH(libartefact,
	[  --with-libartefact      build libartefact (default=no)],
	[], [with_libartefact=no])
if test x$with_libartefact == xyes; then
  DEP_OPENSSL=yes
  DEP_CURL=yes
  DEP_EXPAT=yes

  LIBARTEFACT_PC=libartefact.pc
  AC_CONFIG_FILES(libartefact.pc)
  AC_SUBST(LIBARTEFACT_PC)
fi
AC_CONFIG_FILES(libartefact/Makefile)

######################
# lua scripts support
AC_ARG_WITH(lua,
	[  --with-lua      build lua scripts (default=no)],
	[], [with_lua=no])
if test x$with_lua == xyes; then
  DEP_LUA=yes
fi
AC_CONFIG_FILES(lua/Makefile)

######################
# Barcode extract support
AC_ARG_WITH(barcode,
	[  --with-barcode      build barcode extract (default=no)],
	[], [with_barcode=no])
if test x$with_barcode == xyes; then
  DEP_POSTGRES=yes
fi
AC_CONFIG_FILES(barcodeextract/Makefile)

#########################################
#
#          Dependency checks
#
#########################################

#####################################
# Check for libopenssl (for base 64)
if test x$DEP_OPENSSL == xyes; then
   PKG_CHECK_MODULES(OPENSSL, openssl >= 0.9.7)
fi

#######################
# Check for postgresql
if test x$DEP_POSTGRES == xyes; then
  AC_ARG_WITH(db,
  	[  --with-db               build with db support (default=yes)],
  	[], [with_db=yes])
  if test x$with_db == xno; then
      AC_MSG_WARN([*** Building without db support!])
      AC_DEFINE_UNQUOTED(WITHOUT_DB, , [The project is configured not to use the db])
  else
      PQ_CFLAGS="-I$(pg_config --includedir)"
      AC_SUBST(PQ_CFLAGS)
      PQ_LIBS="-L$(pg_config --libdir) -lpq"
      AC_SUBST(PQ_LIBS)
  fi
fi

#######################
# Check for libartefact
if test x$DEP_LIBARTEFACT == xyes; then
  PKG_CHECK_MODULES([ARTEFACT], [libartefact >= 0.2.0])
fi

##########################
# Check for libmicrohttpd
if test x$DEP_MICROHTTPD == xyes; then
  PKG_CHECK_MODULES(HTTPD, libmicrohttpd >= 0.4.4)
fi

###################
# Check for liblua
if test x$DEP_LUA == xyes; then
  PKG_CHECK_MODULES([LUA], [lua-5.1 >= 5.1 lua-5.1 < 5.2])
  # Create the LUA var i sysconfig.h
  AC_DEFINE_UNQUOTED(LUA_BASEDIR, "$MYPREFIX/share/lua",
   [The path to the lua files])
fi

####################
# Check for libcurl
if test x$DEP_CURL == xyes; then
  PKG_CHECK_MODULES([CURL], [libcurl >= 7.9.5])
fi

####################
# Check for uuid
if test x$DEP_UUID == xyes; then
  PKG_CHECK_MODULES([UUID], [ossp-uuid >= 1.6.1])
fi

dnl ======================
dnl Check for getopt
dnl ======================
AC_HAVE_HEADERS(getopt.h)

##########################
# Check for eXpat library
if test x$DEP_EXPAT == xyes; then
  tmp_CXXFLAGS="$CXXFLAGS"
  tmp_CPPFLAGS="$CPPFLAGS"
  tmp_CFLAGS="$CFLAGS"
  tmp_LDFLAGS="$LDFLAGS"
  tmp_LIBS="$LIBS"
  CXXFLAGS=""
  CPPFLAGS=""
  CFLAGS=""
  LDFLAGS=""
  LIBS=""
  AC_CHECK_HEADER(expat.h, , AC_MSG_ERROR([*** eXpat header file not found!]))
  AC_CHECK_LIB(expat, XML_ParserCreate, , AC_MSG_ERROR([*** eXpat library not found!]))
  EXPAT_CFLAGS="$CXXFLAGS $CPPFLAGS $CFLAGS"
  EXPAT_LIBS="$LDFLAGS $LIBS"
  CXXFLAGS="$tmp_CXXFLAGS"
  CPPFLAGS="$tmp_CPPFLAGS"
  CFLAGS="$tmp_CFLAGS"
  LDFLAGS="$tmp_LDFLAGS"
  LIBS="$tmp_LIBS"
  AC_SUBST(EXPAT_CFLAGS)
  AC_SUBST(EXPAT_LIBS)
fi

################################
# Check for the pthread library
if test x$DEP_PTHREAD == xyes; then
  tmp_CXXFLAGS="$CXXFLAGS"
  tmp_CPPFLAGS="$CPPFLAGS"
  tmp_CFLAGS="$CFLAGS"
  tmp_LDFLAGS="$LDFLAGS"
  tmp_LIBS="$LIBS"
  CXXFLAGS=""
  CPPFLAGS=""
  CFLAGS=""
  LDFLAGS=""
  LIBS=""
  AC_CHECK_HEADER(pthread.h, , AC_MSG_ERROR([*** pthread header file not found!]))
  AC_CHECK_LIB(pthread,  pthread_mutex_init, , AC_MSG_ERROR([*** pthread library not found!]))
  PTHREAD_CFLAGS="$CXXFLAGS $CPPFLAGS $CFLAGS"
  PTHREAD_LIBS="$LDFLAGS $LIBS"
  CXXFLAGS="$tmp_CXXFLAGS"
  CPPFLAGS="$tmp_CPPFLAGS"
  CFLAGS="$tmp_CFLAGS"
  LDFLAGS="$tmp_LDFLAGS"
  LIBS="$tmp_LIBS"
  AC_SUBST(PTHREAD_CFLAGS)
  AC_SUBST(PTHREAD_LIBS)
fi

######################
# Check for flex/yacc
if test x$DEP_CONFIG == xyes; then
  AC_PROG_LEX
  AC_PROG_YACC
fi

AC_CONFIG_FILES(artefact.spec)
AC_CONFIG_FILES(artefact-lua.spec)
AC_CONFIG_FILES(libartefact.spec)
AC_CONFIG_FILES(oftdevproxy.spec)

#######################################
#
#          Component summary
#
#######################################

AM_CONDITIONAL([WITH_OFTDEVPROXY], [test "x$with_oftdevproxy" = "xyes"])
AM_CONDITIONAL([WITH_LIBARTEFACT], [test "x$with_libartefact" = "xyes"])
AM_CONDITIONAL([DEPCONFIG], [test "x$DEP_CONFIG" = "xyes"])
AM_CONDITIONAL([WITH_TEST], [test "x$with_test" = "xyes"])
AM_CONDITIONAL([WITH_ARTEFACT], [test "x$with_artefact" = "xyes"])
AM_CONDITIONAL([WITH_LUA], [test "x$with_lua" = "xyes"])
AM_CONDITIONAL([WITH_BARCODE], [test "x$with_barcode" = "xyes"])

AC_OUTPUT()

AC_MSG_NOTICE([**************************************])
AC_MSG_NOTICE([* Building the following components: *])
AC_MSG_NOTICE([**************************************])
AC_MSG_NOTICE([])

if test x$with_artefact == xyes; then AC_MSG_NOTICE([ - Artefact]); fi
if test x$with_oftdevproxy == xyes; then AC_MSG_NOTICE([ - OftDevProxy]); fi
if test x$with_libartefact == xyes; then AC_MSG_NOTICE([ - LibArtefact]); fi
if test x$with_lua == xyes; then AC_MSG_NOTICE([ - Lua scripts]); fi
if test x$with_barcode == xyes; then AC_MSG_NOTICE([ - Barcode Extract]); fi

AC_MSG_NOTICE([])
