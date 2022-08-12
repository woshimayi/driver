#!/bin/sh

# download SDK and tool
git clone https://gitee.com/EspressifSystems/esp-gitee-tools.git
git clone https://gitee.com/EspressifSystems/esp-idf.git
cd esp-idf/
../esp-gitee-tools/submodule-update.sh 

# install depend lib and python lib
./install.sh esp32

# config PATH
. ./export.sh 

rm -rf ~/.espressif/python_env/idf5.0_py3.7_env
find . -name "*.pyc" | xargs rm -f

# build example
mkdir dof
cp esp-idf/examples/get-started/hello_world/ dof/ -a
cd dof/hello_world/
idf.py set-target esp32
idf.py menuconfig 
idf.py build