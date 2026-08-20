#!/bin/bash

DIR="${1:-.}"

missing=0

for ((i=0; i<=6569; i++)); do
  filename=$(printf "%06d.txt" "$i")
  file="$DIR/$filename"

  if [ ! -f "$file" ]; then
    echo "FALTANDO: $file"
    missing=$((missing + 1))
  fi
done

if [ "$missing" -eq 0 ]; then
  echo "OK: Todos os arquivos existem."
  exit 0
else
  echo "ERRO: $missing arquivo(s) estão faltando."
  exit 1
fi