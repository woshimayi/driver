#!/bin/sh

# download SDK and tool
git clone https://gitee.com/EspressifSystems/esp-gitee-tools.git --depth=1
git clone -b release/v4.3 https://gitee.com/EspressifSystems/esp-idf.git --depth=1
cd esp-idf/
../esp-gitee-tools/submodule-update.sh 

# install depend lib and python lib
./install.sh esp32

# config PATH
. ./export.sh 
echo "alias idf='. /home/zs/Documents/esp32/esp-idf/export.sh; export IOT_SOLUTION_PATH=/home/zs/Documents/esp32/esp-iot-solution/'" >> ~/.bashrc

rm -rf ~/.espressif/python_env/idf5.0_py3.7_env
find . -name "*.pyc" | xargs rm -f

# build example
mkdir dof
cp esp-idf/examples/get-started/hello_world/ dof/ -a
cd dof/hello_world/
idf.py set-target esp32
idf.py menuconfig 
idf.py build
cd ../../

# download esp-iot-solution
git clone --recursive https://github.com/espressif/esp-iot-solution --depth=1
cd esp-iot-solution/
git submodule update --init --recursive
