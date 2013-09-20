#!/bin/bash

function addTests {
    dir=$1
    name=$(echo -n ${dir} | tr [:punct:][:space:] _)
    echo "${name}_TESTS = \\"
    for test in $dir/*.t.cc; do
        echo "    ${test%.cc} \\"
    done
    echo "    \$(NULL)"
    echo "TESTS += \$(${name}_TESTS)"
    echo "check_PROGRAMS += \$(${name}_TESTS)"
    echo
}

function amName {
    echo -n ${1%.cc} | tr [:punct:][:space:] _
}

test_CXXFLAGS="-Icommon -Itools -Ideps/libtap++-0.01"

addTests gearbox/t/core
for test in gearbox/t/core/*.t.cc; do
    name=$(amName $test)
    cat <<EOF
${name}_CXXFLAGS=${test_CXXFLAGS} -DTESTDIR='"\$(abs_top_srcdir)/gearbox/t/core"'
${name}_LDFLAGS=\$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
${name}_LDADD=\$(LIBTAP) gearbox/core/libgearbox_core.la
${name}_SOURCES=$test

EOF
done

cat <<'EOF'
gearbox_t_core_Plugin_t_CXXFLAGS+=-DTESTPLUGINDIR='"$(abs_top_srcdir)/gearbox/t/core/plugins/.libs"' 
gearbox_t_core_PluginAll_t_CXXFLAGS+=-DTESTPLUGINDIR='"$(abs_top_srcdir)/gearbox/t/core/plugins/.libs"' 

check_LTLIBRARIES += gearbox/t/core/plugins/hello.la gearbox/t/core/plugins/goodbye.la
gearbox_t_core_plugins_hello_la_LDFLAGS = -module -rpath /dev/null
gearbox_t_core_plugins_hello_la_LIBADD=gearbox/core/libgearbox_core.la
gearbox_t_core_plugins_hello_la_SOURCES=gearbox/t/core/plugins/TestPlugin.cc gearbox/t/core/plugins/hello.cc

gearbox_t_core_plugins_goodbye_la_LDFLAGS = -module -rpath /dev/null
gearbox_t_core_plugins_goodbye_la_LIBADD=gearbox/core/libgearbox_core.la
gearbox_t_core_plugins_goodbye_la_SOURCES=gearbox/t/core/plugins/TestPlugin.cc gearbox/t/core/plugins/goodbye.cc
EOF

addTests gearbox/t/job
for test in gearbox/t/job/*.t.cc; do
    name=$(amName $test)
    cat <<EOF
${name}_CXXFLAGS=${test_CXXFLAGS} -DTESTDIR='"\$(abs_top_srcdir)/gearbox/t/job"'
${name}_LDFLAGS=\$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
${name}_LDADD=\$(LIBTAP) gearbox/job/libgearbox_job.la
${name}_SOURCES=$test

EOF
done

addTests gearbox/t/store
for test in gearbox/t/store/*.t.cc; do
    name=$(amName $test)
    cat <<EOF
${name}_CXXFLAGS=${test_CXXFLAGS} -DTESTDIR='"\$(abs_top_srcdir)/gearbox/t/store"'
${name}_LDFLAGS=\$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
${name}_LDADD=\$(LIBTAP) gearbox/store/libgearbox_store.la
${name}_SOURCES=$test

EOF
done

addTests gearbox/t/worker
for test in gearbox/t/worker/*.t.cc; do
    name=$(amName $test)
    cat <<EOF
${name}_CXXFLAGS=${test_CXXFLAGS} -DTESTDIR='"\$(abs_top_srcdir)/gearbox/t/worker"'
${name}_LDFLAGS=\$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
${name}_LDADD=\$(LIBTAP) gearbox/worker/libgearbox_worker.la
${name}_SOURCES=$test

EOF
done

cat <<EOF
check_LTLIBRARIES += common/stub/libgearman_stub.la
common_stub_libgearman_stub_la_CXXFLAGS = ${test_CXXFLAGS} \$(LOG4CXX_CFLAGS)
common_stub_libgearman_stub_la_LDFLAGS = -rpath /dev/null -avoid-version \$(LOG4CXX_LIBS) \$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
common_stub_libgearman_stub_la_SOURCES = \
    common/stub/gearman_stub.cc \
    \$(NONE)

EOF

echo "gearbox_t_swig_perl_TESTS = \\"
for test in gearbox/t/swig/perl/*.t; do
    echo "    ${test} \\"
done
echo "    \$(NULL)"
echo
echo "TESTS += \$(gearbox_t_swig_perl_TESTS)"

echo "if WITH_PHP"
echo "gearbox_t_swig_php_TESTS = \\"
for test in gearbox/t/swig/php/*.t; do
    echo "    ${test} \\"
done
echo "    \$(NULL)"
echo
echo "TESTS += \$(gearbox_t_swig_php_TESTS)"
echo "endif"

echo "export PATH=$PATH:\$(abs_top_srcdir)/bin"