AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4 --install

lib_LTLIBRARIES= gearbox/core/libgearbox_core.la

CFLAGS=-g
CXXFLAGS=-g
export DYLD_FORCE_FLAT_NAMESPACE=1

LIBTAP=deps/libtap++-0.01/libtap++.la
check_PROGRAMS = $(TESTS)
check_LTLIBRARIES = $(LIBTAP)

$(LIBTAP): deps/libtap++-0.01/tap++.C
deps/libtap++-0.01/tap++.C:
	cd deps && tar xzf libtap-0.01.tar.gz
# need bogus -rpath to force shared library
deps_libtap___0_01_libtap___la_LDFLAGS = -rpath /dev/null
deps_libtap___0_01_libtap___la_SOURCES = \
	libtap++-0.01/tap++.h \
	deps/libtap++-0.01/tap++.C

TESTS =

TestMakefile.am: auto/testAutoMake.sh
	auto/testAutoMake.sh > TestMakefile.am

include TestMakefile.am

bin_PROGRAMS=gearbox/core/gearbox_json

gearbox_core_libgearbox_core_la_CXXFLAGS = $(YAJL_CFLAGS) $(CURL_CFLAGS) $(BOOST_CPPFLAGS) $(LOG4CXX_CFLAGS) $(UUID_CFLAGS)
gearbox_core_libgearbox_core_la_LDFLAGS = $(YAJL_LIBS) $(CURL_LIBS) $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(LOG4CXX_LIBS) $(BOOST_REGEX_LIB) -lcrypto $(UUID_LIBS) $(BOOST_IOSTREAMS_LIB)
gearbox_core_libgearbox_core_includes = \
	gearbox/core/ConfigFile.h \
	gearbox/core/Errors.h \
	gearbox/core/File.h \
	gearbox/core/Hash.h \
	gearbox/core/HttpClient.h \
	gearbox/core/Json.h \
    gearbox/core/JsonGenerator.h \
    gearbox/core/JsonParser.h \
    gearbox/core/JsonPrivate.h \
    gearbox/core/JsonSchema.h \
    gearbox/core/logger.h \
    gearbox/core/Pipe.h \
    gearbox/core/Plugin.h \
    gearbox/core/REST.h \
    gearbox/core/strlcpy.h \
    gearbox/core/SystemPair.h \
    gearbox/core/TempDir.h \
    gearbox/core/TempFile.h \
    gearbox/core/Uri.h \
    gearbox/core/util.h \
	$(NULL)

gearbox_core_libgearbox_core_la_SOURCES = $(gearbox_core_libgearbox_core_includes) \
    gearbox/core/ConfigFile.cc \
    gearbox/core/Errors.cc \
	gearbox/core/ffdigest.cc \
    gearbox/core/File.cc \
    gearbox/core/Hash.cc \
    gearbox/core/HttpClient.cc \
    gearbox/core/Json.cc \
    gearbox/core/JsonGenerator.cc \
    gearbox/core/JsonParser.cc \
    gearbox/core/JsonSchema.cc \
    gearbox/core/logger.cc \
    gearbox/core/Pipe.cc \
    gearbox/core/Plugin.cc \
    gearbox/core/REST.cc \
    gearbox/core/run.cc \
    gearbox/core/sha1.cc \
    gearbox/core/SystemPair.cc \
    gearbox/core/TempDir.cc \
    gearbox/core/TempFile.cc \
    gearbox/core/Uri.cc \
    gearbox/core/util.cc \
    $(NULL)

gearbox_core_gearbox_json_SOURCES = gearbox/core/gearbox_json.cc
gearbox_core_gearbox_json_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
gearbox_core_gearbox_json_LDADD = gearbox/core/libgearbox_core.la

