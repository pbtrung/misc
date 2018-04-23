#!/bin/bash

pass=""
hkey=""

find "$1" -type f -name "*.e.png" -print0 | while IFS= read -r -d $'\0' line; do
	echo "$line"
	dirpath=$(dirname "$line")
	fn=$(basename "$line" ".e.png")

	decimg="$dirpath/$fn.d.png"
	imgenc -p "$pass" -h "$hkey" -i "$line" -o "$decimg" -d

	rm "$line"
done
