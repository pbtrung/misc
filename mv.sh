#/bin/bash

cd "$1"
n=0
for f in *; do 
	d="$2_$((n++ / $3))"
	mkdir -p "$d"
	mv -- "$f" "$d/$f" 
done
