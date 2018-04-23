#!/bin/bash

encryt() {
	pass=""
	hkey=""

	echo "$1"
	line="$1"
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	fn=${fn::$2}

	encimg="$dirpath/$fn.e.png"
	imgenc -p "$pass" -h "$hkey" -i "$line" -o "$encimg" -e

	rm "$line"
}

find "$1" -type f -iregex '.*\.\(png\|jpg\|gif\|jpeg\)$' -print0 | while IFS= read -r -d $'\0' line; do
	encryt "$line" "-4"
done
