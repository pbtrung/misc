#!/bin/bash

cd "$1"

find . -type f -iregex '.*\.\(zip\|rar\)$' -print0 | while IFS= read -r -d $'\0' line; do
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	fn=${fn::-4}
	echo "FILE: $line"
	echo "FILENAME: $fn"
	aunpack -X "$dirpath/$fn" "$line"
	rm "$line"
done