#!/bin/bash

cd "$1"
find . -type f -name '*.e.png' -print0 | while IFS= read -r -d $'\0' line; do
	echo "$line"
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	new=`echo "$fn" | sed -r 's/[][)(!a-zA-Z.& ]+/t/g' | sed -r 's/\-/t/g'`.e.png
	mv --backup=t "$line" "$dirpath/$new"
done