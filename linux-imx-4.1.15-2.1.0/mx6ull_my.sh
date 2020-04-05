#!/bin/sh

make  distclean
make  imx_my_nand_defconfig
make  menuconfig
make  V=1 -j4
