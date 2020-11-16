### 1：首先解压缩上面的代码,然后配置qemu支持的版本,然后编译

```bash
tar jxvf u-boot-2017.01.tar.bz2
cd u-boot-2017.01
make vexpress_ca9x4_defconfig ARCH=arm
ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- make all -j16
```

### 2：编译成功后,产生三个文件

```bash
"u-boot.bin"  is a raw binary image
"u-boot"      is an image in ELF binary format
"u-boot.srec" is in Motorola S-Record format
```

### 3：qemu 启动 uboot

```bash
qemu-system-arm -M vexpress-a9 -m 512M -kernel u-boot -nographic
```

