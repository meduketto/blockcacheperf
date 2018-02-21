#!/bin/bash
export VERSION=4.15.4
wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-$VERSION.tar.gz
tar xvfz linux-$VERSION.tar.gz
cd linux-$VERSION
make defconfig
make -j8

