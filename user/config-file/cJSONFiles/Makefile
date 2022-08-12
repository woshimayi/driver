




# 执行文件或库文件
LIBREGION = libcjson.so

# 当前目录
CURR_DIR := $(shell pwd)

# 源文件
SRC_DIR = $(wildcard $(SRC)/*.c)

# build 目录
SRC=$(CURR_DIR)/cJSON
BUILD_DIR:=$(subst $(SRC), /hgsApps, $(CURR_DIR))

# 中间文件
# OBJS = $(CURR_DIR)/obj
OBJS = $(patsubst $(SRC)/%.c, $(CURR_DIR)/obj/%.o, $(SRC))

# 依赖库
ALLOWED_LIB_DIRS := /lib

# 头文件搜索路径
ALLOWED_INCLUDE_PATHS := -I.

#安装目录
INSTALL_DIR=$(CURR_DIR)/obj

# 编译链
$(CC)=gcc

# 编译选项
CFLAGS += -shared -fPIC

# 把字串 $(CURR_DIR) 中的 /hgsApps 字符串替换成 /hgsApps
# 返回第一个单词
# BUILD_DIR:=$(word 1, $(BUILD_DIR))

	
all: $(LIBREGION)
	install -m 755 $(LIBREGION) $(INSTALL_DIR)/obj

$(LIBREGION): $(OBJS)
	$(warning "1", $@)
	$(warning "2", $^)
	$(CC) -o $@ $^ $(CFLAGS) 


clean:
	rm -rm bin

$(OBJS):obj/%.o:$(SRC)/%.c
	$(CC) $(CFLAGS) $< $(ALLOWED_INCLUDE_PATHS) -c -o $@
	
.PHONY: all clean	

-include $(COBJS:.o=.d)
-include $(CPPOBJS:.o=.d)
	
%.d: $(SRC)/%.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $(CFLAGS) $< > $@.$$$$ 2>/dev/null; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -M $(CPPFLAGS) $(CFLAGS) $< > $@.$$$$ 2>/dev/null; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
