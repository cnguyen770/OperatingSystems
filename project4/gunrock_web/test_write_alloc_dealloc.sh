#!/bin/bash

cp test_write_alloc_dealloc.img test.img

# Define the target files
file0="file0.txt"
file4096="file4096.txt"
file5000="file5000.txt"
file8192="file8192.txt"
file12288="file12288.txt"

touch $file0
dd if=/dev/zero of="$file4096" bs=4096 count=1
dd if=/dev/zero of="$file5000" bs=5000 count=1
dd if=/dev/zero of="$file8192" bs=4096 count=2
dd if=/dev/zero of="$file12288" bs=4096 count=3

./ds3bits test.img
./ds3ls test.img /

echo -e ">>> Copying "$file0" into dummy.txt"
./ds3cp test.img "$file0" 1
./ds3bits test.img
./ds3cat test.img 1

echo -e ">>> Copying "$file8192" into dummy.txt"
./ds3cp test.img "$file8192" 1
./ds3bits test.img
./ds3cat test.img 1

echo -e "Copying "$file4096" into dummy.txt"
./ds3cp test.img "$file4096" 1
./ds3bits test.img
./ds3cat test.img 1

echo -e "Copying "$file12288" into dummy.txt"
./ds3cp test.img "$file12288" 1
./ds3bits test.img
./ds3cat test.img 1

echo -e "Copying "$file8192" into dummy.txt"
./ds3cp test.img "$file8192" 1
./ds3bits test.img
./ds3cat test.img 1

echo -e "Copying "$file5000" into dummy.txt"
./ds3cp test.img "$file5000" 1
./ds3bits test.img
./ds3cat test.img 1

# Remove the created files
rm "$file0" "$file4096" "$file5000" "$file8192" "$file12288" test.img
