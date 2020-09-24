echo  "$1"

if [ -z "$1" ]
then 
	echo "str fail"
	exit 0
fi

if test $1 = "internet_event_start"
then
	action="-A"
elif test $1 = "internet_event_end"
then 
	action="-D"
fi

g_week=`date | cut -d ' ' -f1`
g_hour=`date | cut -d ' ' -f5 | cut -d ':' -f1`
g_min=`date | cut -d ' ' -f5 | cut -d ':' -f2`

# if [ -z "$g_hour" ]; then
# 	g_hour=`date | cut -d ' ' -f5 | cut -d ':' -f1`
# 	g_min=`date | cut -d ' ' -f5 | cut -d ':' -f2`
# fi

internetTimeTransform(){
	min=0
	hour=0
	tmp_min=0

	if test "48" = $1
	then 
		min=59
		hour=23
		echo $min $hour
		return 0
	fi

	$((tmp_min=$1%2))
	if test "0" = $tmp_min
	then
		min=0
	else
		min=30
	fi

	$((hour=$1/2))
	echo $min $hour
	return 0
}

week_tmp=0
if test $g_week = 'Mon'
then
	week_tmp=1
elif test $g_week = 'Tue'
then
	week_tmp=2
elif test $g_week = 'Web'
then
	week_tmp=3
elif test $g_week = 'The'
then
	week_tmp=4
elif test $g_week = 'Fri'
then
	week_tmp=5
elif test $g_week = 'Sat'
then
	week_tmp=6
elif test $g_week = 'Sun'
then
	week_tmp=0
fi


# if test $action = "-D" ; then
# 	num=0
# 	while test "schedulelist" = `uci get system.@schedulelist[$num]`
# 	do
# 		echo "num=" $num
# 		schedulelist_from=$(internetTimeTransform  `uci get system.@schedulelist[$num].fromTime`)
# 		schedulelist_to=$(internetTimeTransform `uci get system.@schedulelist[$num].toTime`)
# 		echo "$schedulelist_from $schedulelist_to"
# 		echo "week" "`uci get system.@schedulelist[$num].week`" "$week_tmp"
# 		week_result=$(echo "$(uci get system.@schedulelist[$num].week)" | grep "$week_tmp")
# 		echo "result" "$week_result" "$((week_result == week_tmp))"
# 		if ((week_result==week_tmp)) || [[ "$(uci get system.@schedulelist[$num].week)" == "*" ]]; then
# 			echo "wwwwwwwwwwwwwwwww"
# 			schedulelist_from=$(internetTimeTransform "$(uci get system.@schedulelist[$num].fromTime)")
# 			schedulelist_to=$(internetTimeTransform "$(uci get system.@schedulelist[$num].toTime)")
# 			echo "from to" $schedulelist_from $schedulelist_to
# 			start_min=$(echo "$schedulelist_from" | cut -d ' ' -f1)
# 			start_hour=$(echo "$schedulelist_from" | cut -d ' ' -f2)
# 			end_min=$(echo "$schedulelist_to" | cut -d ' ' -f1)
# 			end_hour=$(echo "$schedulelist_to" | cut -d ' ' -f2)
# 			echo 'start_min start_hour end_min end_hour' $start_min $start_hour $end_min $end_hour 
# 			echo $((start_hour==end_hour))
# 			echo $((g_hour>start_hour && g_hour<end_hour))
# 			echo $((g_hour==start_hour && g_min>=start_min))
# 			echo $((g_hour==end_hour && g_min<end_min))

# 			if ((start_hour==end_hour)); then
# 				echo "33333333333333333"
# 				if ((g_hour==start_hour && g_min>=start_min&&g_min<end_min)); then
# 					echo "aaaaaaaaaaaaaaaaaaaaaaaa"
# 					action="-A"
# 					break
# 				fi
# 			elif ((g_hour>start_hour && g_hour<end_hour)); then
# 				echo "dddddddddddddddddddddddd"
# 				action="-A"
# 				break
# 			elif ((g_hour==start_hour && g_min>=start_min)); then
# 				echo "11111111111111111111111"
# 				action="-A"
# 				break
# 			elif ((g_hour==end_hour && g_min<end_min)); then
# 				echo "22222222222222222222222"
# 				action="-A"
# 				break
# 			fi
# 		fi
# 		$((num=num+1))
# 	done
# fi

# echo $action


if test $action = "-A"; then
	ebtables -D FORWARD -p ipv4 --ip-protocol 17 --ip-dport 67 -j ACCEPT 2>>/dev/null
	ebtables -D FORWARD -p ipv4 --ip-protocol 17 --ip-dport 68 -j ACCEPT 2>>/dev/null
	ebtables -D FORWARD -j DROP 2>>/dev/null
	ebtables $action FORWARD -p ipv4 --ip-protocol 17 --ip-dport 67 -j ACCEPT
	ebtables $action FORWARD -p ipv4 --ip-protocol 17 --ip-dport 68 -j ACCEPT
	ebtables $action FORWARD -j DROP
elif test $action = "-D"; then
	ebtables -D FORWARD -p ipv4 --ip-protocol 17 --ip-dport 67 -j ACCEPT 2>>/dev/null
	ebtables -D FORWARD -p ipv4 --ip-protocol 17 --ip-dport 68 -j ACCEPT 2>>/dev/null
	ebtables -D FORWARD -j DROP 2>>/dev/null
fi
