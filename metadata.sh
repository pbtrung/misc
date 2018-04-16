#!/bin/bash

cd "$1"

for dir in *; do
	echo "$dir"
	find "$dir" -type f -name "*.mp3" -print0 | while IFS= read -r -d $'\0' line; do
		echo "$line"
		title=$(basename "$line" ".mp3")
		title=${title/\|/}
		id3v2 -a "author_name" -A "$dir" -t "$title" "$line"
	done
done
