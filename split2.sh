#!/bin/bash

cd "$1"

find . -type f -name "*.mp3" -size +199M -print0 | while IFS= read -r -d $'\0' line; do
	dirpath=$(dirname "$line")
	title=$(basename "$line" ".mp3")
	echo "$line"
	mp3splt -a -t 30.0 -d "$dirpath" -o "$title @n" -g "r%[@o,@N=1,@t=#t @N]" "$line"
	rm "$line"
done