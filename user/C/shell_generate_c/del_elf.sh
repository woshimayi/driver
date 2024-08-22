#!/bin/bash
###
 # @*************************************: 
 # @FilePath     : /user/C/shell_generate_c/del_elf.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2024-08-22 13:17:05
 # @LastEditors  : dof
 # @LastEditTime : 2024-08-22 14:04:37
 # @Descripttion :  delete elf format file
 # @compile      :  
 # @**************************************: 
### 
 

case $1 in
	-l)
		find $2 -name "*" | xargs file | grep ELF | awk -F: '{print $1}'
		;;
	*)
		find $1 -name "*" | xargs file | grep ELF | awk -F: '{print "del " $1}'
		find $1 -name "*" | xargs file | grep ELF | awk -F: '{print $1}' | xargs rm 
esac
