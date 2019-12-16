CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET		  	?= bsp

CC 				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP 		:= $(CROSS_COMPILE)objdump

INCDIRS 		:= imx6ul \
				   bsp/clk \
				   bsp/led \
				   bsp/delay 
				   			   
SRCDIRS			:= project \
				   bsp/clk \
				   bsp/led \
				   bsp/delay 
				   
INCLUDE			:= $(patsubst %, -I %, $(INCDIRS))       # 目录前面加上 -I
$(warning $(INCLUDE))

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))  # foreach 挑选出.S 文件
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
$(warning $(SFILES))
$(warning $(CFILES))

SFILENDIR		:= $(notdir  $(SFILES))
CFILENDIR		:= $(notdir  $(CFILES)) # notdir 去掉路径
$(warning $(SFILENDIR))
$(warning $(CFILENDIR))

SOBJS			:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o)) # 替换
COBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS			:= $(SOBJS) $(COBJS)
$(warning $(SOBJS))
$(warning $(COBJS))
$(warning $(OBJS))


VPATH			:= $(SRCDIRS)

.PHONY: clean
	
$(TARGET).bin : $(OBJS)
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2  $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2  $(INCLUDE) -o $@ $<
	
clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(COBJS) $(SOBJS)

	
