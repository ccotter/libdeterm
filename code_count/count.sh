
c=0
while read x; do
	echo -n "$x: "
	v=`grep -v "^\$" $x | wc -l`
	c=$c" + "$v
	echo $v
done
echo $c
perl -e "print $c"

