# 编译出现问题，可以参考linux makefile

-include config.mk

export MY_VARIABLE       # 使用 export 关键字，将变量传递给所有子 make 进程

PWD := $(shell pwd)

# 包含编译的子目录
obj-m += iptables_trace/
obj-m += iptables_trace/
obj-m += netfilter_trace/  
obj-m += subnet_icmp/
obj-m += kernel_time/    
obj-m += netfilter-br/  
obj-m += proc_net/    
obj-m += sub_protol/

# 默认目标：编译模块
all:
	$(MAKE) -C $(KER_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules

# 其他目标：安装、清理等，也需要委托给内核构建系统
install:
	$(MAKE) -C $(KER_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KER_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean