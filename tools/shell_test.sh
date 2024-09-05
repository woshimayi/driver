###
 # @*************************************: 
 # @FilePath     : /tools/shell_test.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2024-08-16 11:10:09
 # @LastEditors  : dof
 # @LastEditTime : 2024-08-16 11:10:10
 # @Descripttion :  
 # @compile      :  
 # @**************************************: 
### 



branch=$1

case "${branch}" in
    'b19')
        echo "item = 1"
    ;;
    'b16'|'b15')
        echo "item = 2 or item = 3"
    ;;
    *)
        echo "default (none of above)"
    ;;
esac
