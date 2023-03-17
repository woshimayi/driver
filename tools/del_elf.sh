# delete elf format file

case $1 in
	-l)
		find $2 -name "*" | xargs file | grep ELF | awk -F: '{print $1}'
		;;
	*)
		find $1 -name "*" | xargs file | grep ELF | awk -F: '{print "del " $1}'
		find $1 -name "*" | xargs file | grep ELF | awk -F: '{print $1}' | xargs rm 
esac
