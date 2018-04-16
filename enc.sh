#!/bin/bash

encryt() {
	pass="xxx"
	hkey="yyy"

	echo "$1"
	line="$1"
	dirpath=$(dirname "$line")
	fn=$(basename "$line")
	fn=${fn::$2}
	ppm="$dirpath/$fn.raw.ppm"
	"$3" "$line" > "$ppm"

	iv=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 50 | head -n 1)
	hiv=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 50 | head -n 1)
	enciv="$dirpath/iv.txt"
	echo "iv $iv" > "$enciv"
	echo "hiv $hiv" >> "$enciv"

	header="$dirpath/header.txt"
	head -n 3 "$ppm" > "$header"
	ppmbody="$dirpath/ppm.body.bin"
	tail -n +4 "$ppm" > "$ppmbody"

	encbody="$dirpath/ppm.body.enc"
	imgenc -p "$pass" -s "$iv" -i "$ppmbody" > "$encbody"
	encppm="$dirpath/$fn.ppm"
	cat "$header" "$encbody" > "$encppm"

	enchkey=`imgenc -h "$hkey" -s "$hiv"`
	hmac=`echo "$iv$hiv" | cat - "$encppm" | openssl dgst -blake2b512 -hex -hmac "$enchkey" | sed 's/^.*= //'`
	echo "hmac $hmac" >> "$enciv"
	encpng="$dirpath/$fn.e.png"
	pnmtopng -text="$enciv" "$encppm" > "$encpng"

	rm "$ppm"
	rm "$encppm"
	rm "$line"
	rm "$dirpath"/*.txt
	rm "$ppmbody"
	rm "$encbody"
}

find "$1" -type f -exec file --no-pad --mime-type {} + | grep 'image/png' | sed 's/: image\/png//' | while IFS= read -r -d $'\n' line; do
	echo "image/png"
	encryt "$line" "-4" "pngtopnm"
done
find "$1" -type f -exec file --no-pad --mime-type {} + | grep 'image/jpeg' | sed 's/: image\/jpeg//' | while IFS= read -r -d $'\n' line; do
	echo "image/jpeg"
	encryt "$line" "-4" "jpegtopnm"
done
find "$1" -type f -exec file --no-pad --mime-type {} + | grep 'image/gif' | sed 's/: image\/gif//' | while IFS= read -r -d $'\n' line; do
	echo "image/gif"
	encryt "$line" "-4" "giftopnm"
done
