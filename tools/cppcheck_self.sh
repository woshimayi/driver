#!/bin/bash


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

if [ ! -d "$outputdir" ]
then
	mkdir $outputdir
fi

# --source-dir  源码目录
# --title       标题目录
# --file        扫描生成结果目录
# --report-dir  生成html 目录

$execpath --source-dir=./ --title=cppcheck_$path --file=$reprot --report-dir=$outputdir

rm $reprot


