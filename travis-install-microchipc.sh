#!/bin/sh
set -ex
wget http://ww1.microchip.com/downloads/en/DeviceDoc/xc8-v1.34-full-install-linux-installer.run
chmod 755 xc8-v1.34-full-install-linux-installer.run
sudo ./xc8-v1.34-full-install-linux-installer.run
