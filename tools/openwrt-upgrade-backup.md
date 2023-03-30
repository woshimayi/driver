Openwrt 分区、备份恢复与升级

Openwrt 分区、备份恢复与升级
Openwrt 分区
openwrt 启动时分区如下
修改openwrt 分区和读写属性
备份与恢复
备份和恢复用户修改数据和系统配置数据
仅备份和恢复系统配置数据
恢复出厂设置
升级
tftp命令uboot模式下升级
mtd命令升级
sysupgrade 升级

Openwrt 分区
openwrt 启动时分区如下

```shell
[    0.720000] Creating 6 MTD partitions on "spi0.0":
[    0.730000] 0x000000000000-0x000000040000 : "u-boot"
[    0.730000] 0x000000040000-0x000000050000 : "u-boot-env"
[    0.740000] 0x000000050000-0x000000e80000 : "rootfs"
[    0.750000] mtd: partition "rootfs" set to be root filesystem
[    0.750000] mtd: partition "rootfs_data" created automatically, ofs=960000, len=520000 
[    0.760000] 0x000000960000-0x000000e80000 : "rootfs_data"
[    0.770000] 0x000000e80000-0x000000ff0000 : "kernel"
[    0.780000] 0x000000ff0000-0x000001000000 : "art"
[    0.780000] 0x000000050000-0x000000ff0000 : "firmware"
```

修改openwrt 分区和读写属性

```shell
vi qsdk/target/linux/ar71xx/image/Makefile
ap143_mtdlayout_16M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot- env),14528k(rootfs),1472k(kernel),64k(art),16000k@0x50000(firmware)
```



备份与恢复
备份和恢复用户修改数据和系统配置数据

# 备份

```shell
dd  if=/dev/mtd3 of=/tmp/overlay.bin
```

# 恢复

```
mtd -r write /tmp/overlay.bin rootfs_data 
```

仅备份和恢复系统配置数据





### openwrt 自带命令备份升级

# 备份

```
sysupgrade -b /tmp/bak.tar.gz
```

# 恢复

```
sysupgrade -r /tmp/bak.tar.gz
```



恢复出厂设置
# 擦除rootfs_data 

```shell
mtd -r erase rootfs_data
```

# 删除overlay

```shell
rm -rf /overlay/* && reboot
```

## 升级
tftp命令uboot模式下升级

```shell
tftp 0x80060000 ***-uboot.bin && erase 0x9f000000 +0x30000 && cp.b $fileaddr 0x9f000000 $filesize
tftp 0x80060000 ***-kernel.bin && erase 0x9fe80000 +${filesize} && cp.b $fileaddr 0x9fe80000 0x160000
tftp 0x80060000 ***-rootfs.bin && erase 0x9f050000 +${filesize} && cp.b $fileaddr 0x9f050000 $filesize
```

### sysupgrade 命令升级
查看分区情况

```shell
cat /proc/mtd
dev:    size   erasesize  name
mtd0: 00040000 00010000 "u-boot"
mtd1: 00010000 00010000 "u-boot-env"
mtd2: 00e30000 00010000 "rootfs"
mtd3: 00520000 00010000 "rootfs_data"
mtd4: 00170000 00010000 "kernel"
mtd5: 00010000 00010000 "art"
mtd6: 00fa0000 00010000 "firmware"

dd if=/dev/mtd6 of=/tmp/firmware.bin
mtd -r write /tmp/firmware.bin firmware
```

### sysupgrade 升级 

```shell
sysupgrade -r ***-sysupgrade.bin 
```



