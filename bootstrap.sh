#!/usr/bin/env bash

# This is an non-interactive session
export DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# UPDATE PACKAGE LIST 1
# -----------------------------------------------------------------------------

# Assume Yes to everything (and do it quietly)
apt-get --quiet --yes --force-yes update

# -----------------------------------------------------------------------------
# ADD NON STANDARD REPOSITORIES
# -----------------------------------------------------------------------------

# Install python and software properties, needed for add-apt-repository
apt-get --quiet --yes --force-yes install python-software-properties

# Add GNU ARM Embedded Toolchain repository
add-apt-repository --yes ppa:terry.guo/gcc-arm-embedded

# -----------------------------------------------------------------------------
# UPDATE PACKAGE LIST 2
# -----------------------------------------------------------------------------

# Assume Yes to everything (and do it quietly)
apt-get --quiet --yes --force-yes update

# -----------------------------------------------------------------------------
# INSTALL PACKAGES
# -----------------------------------------------------------------------------

# Install GCC arm toolchain
apt-get --quiet --yes --force-yes install gcc-arm-none-eabi

# Install Doxygen and Grapviz package for source code documentation
apt-get --quiet --yes --force-yes install doxygen
apt-get --quiet --yes --force-yes install graphviz
apt-get --quiet --yes --force-yes install make
