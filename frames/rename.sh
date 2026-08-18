#!/bin/bash

for file in BA*.txt; do
  number="${file#BA}"
  number="${number%.txt}"

  new_number=$(printf "%04d" "$number")

  new_file="BA${new_number}.txt"

  if [ "$file" != "$new_file" ]; then
    mv -- "$file" "$new_file"
    echo "$file -> $new_file"
  fi
done