#!/bin/bash

encryt() {
	pass="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	hkey="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

	echo "$1"
	line="$1"
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	fn=${fn::$2}

	iv=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 50 | head -n 1)
	hiv=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 50 | head -n 1)

	encimg="$dirpath/$fn.e.png"
	imgenc -p "$pass" -s "$iv" -i "$line" -o "$encimg"

	enchkey=`imgenc -h "$hkey" -s "$hiv"`
	hmac=`echo "$iv$hiv" | cat - "$encimg" | openssl dgst -blake2b512 -hex -hmac "$enchkey" | sed 's/^.*= //'`
	exiftool -overwrite_original -comment="$iv$hiv$hmac" "$encimg"

	rm "$line"
}

find "$1" -type f -iregex '.*\.\(png\|jpg\|gif\|jpeg\)$' -print0 | while IFS= read -r -d $'\0' line; do
	encryt "$line" "-4"
done
