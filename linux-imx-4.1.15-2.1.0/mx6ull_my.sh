#!/bin/sh

make  distclean
make  imx_dof_nand_defconfig
make  menuconfig
make  V=1 -j4
make  dtbs
