#!/bin/bash

find "$1" -mindepth 2 -type d -print0 | while IFS= read -r -d $'\0' line; do
	parent=$(dirname "$line")
	num=`find "$parent" -mindepth 1 -maxdepth 1 -type d | wc -l`
	
	if [ "$num" = "1" ]; then
		mv --backup=t "$line"/* "$parent"
    fi
done

find "$1" -type d -empty -delete

find "$1" -type f -print0 | while IFS= read -r -d $'\0' line; do
	dir1=$(dirname "$line")
	dir2=$(dirname "$dir1")
	num=`find "$dir2" -mindepth 1 -maxdepth 1 -type d | wc -l`
	
	if [ "$num" = "1" ]; then
		mv --backup=t "$line" "$dir2"
    fi
done

find "$1" -type d -empty -delete	