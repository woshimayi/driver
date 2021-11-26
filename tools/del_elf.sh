# delete elf format file
find  | xargs file | grep ELF | awk -F: '{print $1}'
find $1 -name "*" | xargs file | grep ELF | awk -F: '{print $1}' | xargs rm 
