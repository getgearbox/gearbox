#!/bin/bash

function addTests {
    dir=$1
    echo "TESTS += \\"
    for test in $dir/*.t.cc; do
        echo "    ${test%.cc} \\"
    done
    echo "    \$(NULL)"
    echo
}

function amName {
    echo -n ${1%.cc} | tr [:punct:][:space:] _
}

test_CXXFLAGS="-Icommon -Itools -Ideps/libtap++-0.1"

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
