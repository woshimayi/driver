###
# @*************************************:
# @FilePath: /tools/git_build.sh
# @version:
# @Author: dof
# @Date: 2023-08-15 20:27:43
# @LastEditors: dof
# @LastEditTime: 2023-08-15 20:43:35
# @Descripttion: 循环编译git 版本
# @**************************************:
###


while true
do
    git_commit=$(git rev-parse HEAD)
    
    git_id=$(wget -q http://xx.xx.xx.xx:80xx/f89b7664xe70b5xc31ab3e76x97d9bxdcbx376d -O -)
    
    if [ -d "samba/$git_id" ]; then
        echo "samba/$git_id already exists"
        continue
    fi
    echo $git_id "\t" $git_commit
    # python3 build/build.py -c xiling
    # build_test.sh xiling
    git reset HEAD^
done
