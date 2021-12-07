# delete elf format file
find $1 -name "*" | xargs file | grep ELF | awk -F: '{print $1}'
find $1 -name "*" | xargs file | grep ELF | awk -F: '{print $1}' | xargs rm 
