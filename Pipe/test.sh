#!/bin/bash

ls | cat > compare.txt

./connect ls : cat > dest.txt

sort compare.txt | cat > compare1.txt
sort dest.txt | cat > dest1.txt

comm -23 compare1.txt dest1.txt > difference.txt

if [[ $(tr -d "\r\n" < difference.txt|wc -c) -eq 0 ]]; then
    echo "Test Passed"
else
    echo "Test Failed"
fi

rm -rf *.txt

exit 0