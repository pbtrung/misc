#!/bin/bash

cd "$1"
touch "$2"
for dir in *; do
	new=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)
	mv --backup=t "$dir" "$new"
	echo "$new    $dir" >> "$2"
done
