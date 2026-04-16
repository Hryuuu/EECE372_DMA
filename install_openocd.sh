#!/bin/sh
set -e

# Install OpenOCD for programming the MSPM0C1104 using the XDS110 debug probe. (rpiz2)
sudo apt update
sudo apt install -y \
    git build-essential autoconf automake libtool pkg-config libusb-1.0-0-dev libhidapi-dev

git clone https://github.com/openocd-org/openocd.git
cd openocd
git submodule update --init --recursive

./bootstrap
./configure --enable-internal-jimtcl --enable-xds110
make -j"$(nproc)"
sudo make install