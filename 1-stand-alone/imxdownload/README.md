### 1: imxdownload 为imx6ull 裸机和uboot 添加烧录头，方便调试

```bash
imxdown xxx.bin   #	生成load.imx 文件 
```

### 2：windows 使用

通过 mftools 工具

### 3：linux 使用 uuu 进行烧录

​	[https://github.com/NXPmicro/mfgtools/releases](https://github.com/NXPmicro/mfgtools/releases)

```bash
sudo uuu load.imx    # 下载到板子内存中运行
```

### 4： 使用

```bash
cd obj
cmake ../
make 
make install
```

 