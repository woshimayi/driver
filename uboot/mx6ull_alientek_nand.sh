make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- mx6ull_14x14_ddr256_nand_defconfig
make V=1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4
