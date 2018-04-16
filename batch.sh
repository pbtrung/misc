#!/bin/bash

cd "$1"
while IFS= read -r line; do
	rclone -v --stats=5s copy abc:"xyz/$line" .
done < list.txt

axel -o 'xyz.zip' '[URL]'

for f in *.zip; do
	unzip "$f"
	rm "$f"
done

find . -type f -exec mv --backup=t {} "$2" \;

cd "$2"
find . -type f -iregex '.*\.\(zip\|rar\)$' -print0 | while IFS= read -r -d $'\0' line; do
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	fn=${fn::-4}
	echo "FILE: $line"
	echo "FILENAME: $fn"
	aunpack -X "$dirpath/$fn" "$line"
	rm "$line"
done

systemctl poweroff
