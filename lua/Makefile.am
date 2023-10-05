AUTOMAKE_OPTIONS = subdir-objects

if WITH_TEST
bin_PROGRAMS = test_lua
endif

EXTRA_DIST = \
	ark700a.lua \
	barcode.lua \
	base64.lua \
	cpr.lua \
	cpr_pracroxml.lua \
	echo.lua \
	iolmaster500.lua \
	la350.lua \
	lensmeter.lua \
	lensmeter_pracroxml.lua \
	lensmeter_validator.lua \
	lensmeter_xml.lua \
	lm600p.lua \
	nt3000.lua \
	raw.lua \
	referral.lua \
	referral_pracroxml.lua \
	test.lua \
	tonoref2.lua \
	utils.lua \
	visutron900plus.lua

datadir=${prefix}/usr/share/artefact
if WITH_LUA
dist_data_DATA = \
	ark700a.lua \
	barcode.lua \
	base64.lua \
	cpr.lua \
	cpr_pracroxml.lua \
	echo.lua \
	iolmaster500.lua \
	la350.lua \
	lensmeter.lua \
	lensmeter_pracroxml.lua \
	lensmeter_validator.lua \
	lensmeter_xml.lua \
	lm600p.lua \
	nt3000.lua \
	raw.lua \
	referral.lua \
	referral_pracroxml.lua \
	test.lua \
	tonoref2.lua \
	utils.lua \
	visutron900plus.lua
endif

test_lua_LDADD = $(CONFIG_LIBS) $(LUA_LIBS) $(CPPUNIT_LIBS) 

test_lua_CXXFLAGS = -I../lib $(LUA_CFLAGS) -I../tools/ $(CONFIG_CXXFLAGS) \
  $(CPPUNIT_CFLAGS) -I$(top_srcdir)/hugin

LUALIBS = \
  ../lib/luautil.cc \
  ../lib/lua.cc

test_lua_SOURCES = \
  $(LUALIBS) \
  $(top_srcdir)/hugin/hugin.c \
  test_lua.cc 
