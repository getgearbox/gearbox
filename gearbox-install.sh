#!/bin/bash

# This script installs the gearbox rpms. The PKG_VERSION and PKG_RELEASE environment
# variables need to be set appropriately.

sudo yum install -y *gearbox*-$PKG_VERSION-$PKG_RELEASE.el6.x86_64.rpm
