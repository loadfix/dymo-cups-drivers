#!/bin/bash

# Exit on any error
set -e

autoscan

aclocal
autoconf

autoheader
automake --add-missing
automake

./configure

make

