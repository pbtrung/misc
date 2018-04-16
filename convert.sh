#!/bin/bash

cd "$1"

find . -type f -name "*.m4*" -print0 | while IFS= read -r -d $'\0' line; do
	echo "$line"
	output=${line%.m4*}.mp3
	echo "$output"
	ffmpeg -nostdin -i "$line" "$output"
	# dirpath=$(dirname "$output")
	# echo "$dirpath"
	# mp3splt -a -t 30.0 -d "$dirpath" -o "@n" -g "r%[@o,@N=1,@t=#t @N]" "$output"
done
