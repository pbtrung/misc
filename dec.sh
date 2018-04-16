#!/bin/bash

pass="xxx"
hkey="yyy"

find "$1" -type f -name "*.e.png" -print0 | while IFS= read -r -d $'\0' line; do
	echo "$line"
	dirpath=$(dirname "$line")
	fn=$(basename "$line" ".e.png")
	encppm="$dirpath/$fn.e.ppm"
	enciv="$dirpath/iv.txt"
	pngtopnm -text="$enciv" "$line" > "$encppm"

	iv=""
	hiv=""
	hmac=""
	while IFS=' ' read lhs rhs
	do
		if [ "$lhs" = "iv" ]; then
			iv=$rhs
    	fi
    	if [ "$lhs" = "hiv" ]; then
			hiv=$rhs
    	fi
    	if [ "$lhs" = "hmac" ]; then
			hmac=$rhs
    	fi
	done < "$enciv"

	enchkey=`imgenc -h "$hkey" -s "$hiv"`
	encppmhmac=`echo "$iv$hiv" | cat - "$encppm" | openssl dgst -blake2b512 -hex -hmac "$enchkey" | sed 's/^.*= //'`
	if [ "$encppmhmac" != "$hmac" ]; then
		echo ""
		echo "ERROR: HMAC"
		echo "FILE: $line"
		echo ""
		exit -1
	fi

	header="$dirpath/header.txt"
	head -n 3 "$encppm" > "$header"
	encbody="$dirpath/ppm.body.enc"
	tail -n +4 "$encppm" > "$encbody"

	ppmbody="$dirpath/ppm.body.bin"
	imgenc -p "$pass" -s "$iv" -i "$encbody" > "$ppmbody"
	ppm="$dirpath/$fn.ppm"
	cat "$header" "$ppmbody" > "$ppm"
	img="$dirpath/$fn.d.png"
	pnmtopng "$ppm" > "$img"

	rm "$ppm"
	rm "$encppm"
	rm "$line"
	rm "$dirpath"/*.txt
	rm "$ppmbody"
	rm "$encbody"
done
