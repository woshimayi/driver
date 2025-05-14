#!/bin/bash

###
# @*************************************:
 # @FilePath     : /tools/inotify_backup.sh
# @version      :
# @Author       : dof
# @Date         : 2025-01-24 16:11:35
 # @LastEditors  : dof
 # @LastEditTime : 2025-01-24 17:46:14
# @Descripttion :  inotify real time sync file
# @compile      :
# @**************************************:
###

#要同步的目录或文件列表
SRC_LIST=(
    "/home/zsD/Documents/wget/GPXSee"
)

SERVERIP="172.16.36.35"
USER="hg-work"
PASS="zxcv"
PASSFILE="/opt/rsync.password"
MODULE_NAME="~/Downloads"

LOCALIP=$(hostname -I | awk '{print $1}')

DEST=~/Downloads/${LOCALIP}/${STRING_SRC}
TAR_DIR=~/Downloads/tar

#把目录或者文件拷贝到/opt/${LOCALIP}/下，
#每隔60秒检查2个文件夹是否有差异，
#有差异
#同步代码到/opt/10.0.0.11/下，
#把代码打包推送到远端
#无差异则不做任何操作

#拷贝源目录到对比目录
for SRC in ${SRC_LIST[@]}
do
    #/tmp/test1变为_tmp_test1,用于文件名
    # STRING_SRC=$(echo $SRC | sed 's#/#_#g')
    STRING_SRC=$(basename $SRC)
    DIR=~/Downloads/${LOCALIP}
    DEST=${DIR}/${STRING_SRC}
    [ ! -e $DEST ] && {
        mkdir -p $DIR
        cp -r $SRC $DEST
    }
done

#持续对比
while true; do
    sleep 60
    for SRC in ${SRC_LIST[@]}; do
        # STRING_SRC=$(echo $SRC | sed 's#/#_#g')
        STRING_SRC=$(basename $SRC)
        #用于判断文件夹是否有变化
        lines=$(rsync -avz --dry-run $SRC $DEST | awk '/^sending/,/^sent/' | wc -l)
        echo "lines= $lines"
        #大于3就代表有变化
        if [ $lines -gt 3 ]; then
            rsync -avz $SRC $DEST
            time=$(date +%Y%m%d-%H%M%S)
            mkdir -p $TAR_DIR && cd $TAR_DIR
            tarName=$LOCALIP${STRING_SRC}-${time}.tar.gz
            tar -cvf ${tarName} $DEST
            sshpass -p $PASS rsync -avz  ${tarName} ${USER}@${SERVERIP}:${MODULE_NAME}
        fi
    done
done
EOF
