#!/bin/bash

make clean
make all

for filename in tests/*; do
  $filename >> "$filename"".out"
done

for filename in tests/*; do
  if [[ "$filename" =~ test.*[^v].out ]]; then
    a="$filename"
    verify_file=${a/".out"/"_v.out"}
    echo "-------------------DIFFing "$filename"-------------------"
    b=$(diff "$filename" "$verify_file")
    if [ -z "$b" ] 
    then
      echo "---------------------PASS----------------------------"
    else
      echo "---------------------FAIL----------------------------"; echo "$b"
    fi
  fi
done
