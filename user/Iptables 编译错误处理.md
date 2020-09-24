### 问题一：

```c
Make 出现Sgml2html  not found 时
安装：sudo apt install linuxdoc-tools
Sgml2html 
```

### 问题二：

```c
./configure: line 12781: syntax error near unexpected token `libmnl,'

./configure: line 12781: `      PKG_CHECK_MODULES(libmnl, libmnl >= 1.0, mnl=1, mnl=0)'
```

解决方法：注释掉

```c
\#PKG_CHECK_MODULES(libmnl, libmnl >= 1.0, mnl=1, mnl=0)
添加
mnl=1
```

其他PKG_CHECK_MODULES 出现问题时类似处理