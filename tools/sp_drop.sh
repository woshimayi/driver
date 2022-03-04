###
 # @*************************************: 
 # @FilePath: /network/home/zs/Documents/driver/tools/sp_drop.sh
 # @version: 
 # @Author: dof
 # @Date: 2022-02-25 17:13:26
 # @LastEditors: dof
 # @LastEditTime: 2022-02-26 16:56:49
 # @Descripttion: 
 # @**************************************: 
### 

# a=$(bs /b/e egress_tm/dir=us,index=1 | grep queue_stat | awk -F '=' '{print $9}' | awk -F ',' '{printf $1 " "}' | read queue_1 queue_2 queue_3 queue_4)




while true
do 
	a=$(bs /b/e egress_tm/dir=us,index=1 | grep queue_stat | awk -F '=' '{print $9}' | awk -F ',' '{printf $1 " "}')
	echo "zzzzz $a enter <<<"

	q=0
	p=7
	for i in $a
	do
		echo "zzzz enter $i, loop = $q, p = $p"
		if [ $i != 0 ]
		then
			echo bs /b/c egress_tm/dir=us,index=1 queue_cfg\[$q\]={queue_id=$p,drop_threshold=512}
			echo bs /b/c egress_tm/dir=us,index=1 queue_cfg\[$q\]={queue_id=$p,drop_threshold=1047552}
			echo "zzzzz for success"
			# sleep 4
		fi
		q=$(expr $q + 1 )
		p=$(expr $p - 1 )
	done
	sleep 1
done



