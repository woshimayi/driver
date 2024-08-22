#!/bin/bash
###
 # @*************************************: 
 # @FilePath     : /user/C/shell_generate_c/cppcheck_self.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2024-08-22 16:25:22
 # @LastEditors  : dof
 # @LastEditTime : 2024-08-22 19:00:17
 # @Descripttion :  
 # @compile      :  
 # @**************************************: 
### 


path=$1
execpath=/usr/bin/cppcheck-htmlreport                 # 使用cppcheck-htmlreport 脚本进行扫描
reprot=report-src.xml                                 # 中间文件
outputdir=cppcheck_dir                                # 生成html结果文件夹

# --enable             生成所有的提示
# --suppress           忽略包含警告
# --xml --xml-version  使用xml 格式文件
# $path                扫描文件
# report-src.xml       导出结果到report-src.xml

cppcheck --enable=all --check-config --suppress=missingIncludeSystem   --xml --xml-version=2 $path  2>$reprot


# inlcude=$(find $path -name *.h | xargs dirname | awk '{print "-I " $1}')


rm -rf $outputdir
if [ ! -d "$outputdir" ]
then
	mkdir $outputdir
fi


# inlcude=-include=user/C/http-master \
		# -include=user/C/curl/__MACOSX/cJSON \
		# -include=user/C/curl/cJSON \
		# -include=user/C/log/log \
		# -include=user/C/string \
		# -include=user/C/cmake_test/src \
		# -include=user/C/fileIO/__MACOSX/cJSON \
		# -include=user/C/fileIO/cJSON \
		# -include=user/C/ipc_shm \
		# -include=user/C/123_call \
		# -include=user/C/FileIO/read-ini 

echo "inlcude $inlcude"

# --source-dir  源码目录
# --title       标题目录
# --file        扫描生成结果目录
# --report-dir  生成html 目录

$execpath --source-dir=./ $inlcude --title=cppcheck_$path --file=$reprot --report-dir=$outputdir

rm $reprot


