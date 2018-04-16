#!/bin/bash

cd "$1"

for f in *.zip; do
	unzip "$f"
	rm "$f"
done

systemctl poweroff
