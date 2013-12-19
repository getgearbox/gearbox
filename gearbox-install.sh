#!/bin/bash

# This script installs the gearbox rpms. The PKG_VERSION and PKG_RELEASE environment
# variables need to be set appropriately.

LIST_RPM=("core" "perl-lib" "daemons" "test-workers-config" "job-devel" "job" "worker" \
          "status-plugin-sql" "svc-builder" "daemons-config" "test-workers" "tools" \
          "store-devel" "store" "dbsetup" "python-lib" "worker-devel" "core-devel")

for rpm_suffix in ${LIST_RPM[@]}; do
    sudo yum install -y gearbox-$rpm_suffix-$PKG_VERSION-$PKG_RELEASE.el6.x86_64
done
sudo yum install -y mod_gearbox-$PKG_VERSION-$PKG_RELEASE.el6.x86_64
