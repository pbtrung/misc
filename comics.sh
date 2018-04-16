#!/bin/bash

cd "$1"

for dir in *; do
	find "$dir" -type f -name "*.CBZ" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".CBZ")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
	find "$dir" -type f -name "*.CBR" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".CBR")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
	find "$dir" -type f -name "*.cbz" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".cbz")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
	find "$dir" -type f -name "*.cbr" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".cbr")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
	find "$dir" -type f -name "*.zip" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".zip")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
	find "$dir" -type f -name "*.rar" -print0 | while IFS= read -r -d $'\0' line; do
		dirpath=$(dirname "$line")
		title=$(basename "$line" ".rar")
		echo "FILE: $line"
		echo "PATH: $dirpath"
		echo "NAME: $title"
		aunpack -X "$dirpath/$title" "$line"
		rm "$line"
	done
done