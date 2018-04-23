#!/bin/bash

pass="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
hkey="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

find "$1" -type f -name "*.e.png" -print0 | while IFS= read -r -d $'\0' line; do
	echo "$line"
	dirpath=$(dirname "$line")
	fn=$(basename "$line" ".e.png")

	comment=`exiftool -s -S -comment "$line"`
	iv=${comment:0:50}
	hiv=${comment:50:50}
	hmac=${comment:100}
	exiftool -overwrite_original -comment= "$line"

	enchkey=`imgenc -h "$hkey" -s "$hiv"`
	enchmac=`echo "$iv$hiv" | cat - "$line" | openssl dgst -blake2b512 -hex -hmac "$enchkey" | sed 's/^.*= //'`
	if [ "$enchmac" != "$hmac" ]; then
		echo ""
		echo "ERROR: HMAC"
		echo "FILE: $line"
		echo ""
		exit -1
	fi

	decimg="$dirpath/$fn.png"
	imgenc -p "$pass" -s "$iv" -i "$line" -o "$decimg"

	rm "$line"
done
