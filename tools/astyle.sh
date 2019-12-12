for f in $(find $1 -name  '*.c' -or -name '*.cpp' -or -name '*.h' -type f)
do
astyle --style=allman -k3 -W1 -xG -S -s4 -xb -U -p -xf -xh -xC120 -xL -H -Y -xW -w -n $f
done

