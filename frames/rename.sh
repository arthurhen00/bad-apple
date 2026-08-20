#!/bin/bash

for file in BA[0-9][0-9][0-9][0-9].txt; do
  number="${file#BA}"
  number="${number%.txt}"

  new_number=$((10#$number - 1))
  new_file=$(printf "%06d.txt" "$new_number")

  if [ "$file" != "$new_file" ]; then
    mv -- "$file" "$new_file"
    echo "$file -> $new_file"
  fi
done