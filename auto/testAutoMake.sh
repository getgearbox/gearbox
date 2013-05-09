#!/bin/bash

echo "TESTS += \\"
for test in gearbox/t/core/*.t.cc; do
    echo "    ${test%.cc} \\"
done
echo "    \$(NULL)"
echo

for test in gearbox/t/core/*.t.cc; do
    name=$(echo -n ${test%.cc} | tr [:punct:][:space:] _)
    cat <<EOF
${name}_CXXFLAGS=-Itools -Ideps/libtap++-0.1 -DTESTDIR='"gearbox/t/core"'
${name}_LDFLAGS= \$(BOOST_LDFLAGS) \$(BOOST_SYSTEM_LIB)
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

