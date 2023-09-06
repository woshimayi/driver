SET(LIBHELLO_SRC hello.c)													# 静态库源文件

ADD_LIBRARY(hello SHARED ${LIBHELLO_SRC})									# 生成动态库
SET_TARGET_PROPERTIES(hello PROPERTIES VERSION 1.2 SOVERSION 1)				# 动态库版本号


ADD_LIBRARY(hello_static STATIC ${LIBHELLO_SRC})							# 生成静态库
SET_TARGET_PROPERTIES(hello_static PROPERTIES OUTPUT_NAME "hello")			# 生成静态库和动态库同名的静态库


INSTALL(TARGETS hello hello_static
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)											# 静态库使用 ARCHIVE

INSTALL(FILES hello.h DESTINATION include/hello)
