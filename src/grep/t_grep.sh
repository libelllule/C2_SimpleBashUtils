#!/bin/bash

TESTS=(
    "grep \"apple\" test1.txt"
    "grep -i \"apple\" test1.txt"
    "grep -v \"apple\" test1.txt"
    "grep -c \"apple\" test1.txt"
    "grep -n \"apple\" test1.txt"
    "grep -l \"apple\" test1.txt test2.txt"
    "grep -o \"apple\" test1.txt"
    "grep -e \"apple\" -e \"123\" test1.txt"
    "grep -f patterns.txt test1.txt"
    "grep -iv \"apple\" test1.txt"
    "grep -cv \"apple\" test1.txt"
    "grep -niv \"apple\" test1.txt"
    "grep -oi \"apple\" test1.txt"
    "grep \"apple\" test1.txt test2.txt"
    "grep -h \"apple\" test1.txt test2.txt"
    "grep -n \"apple\" test1.txt test2.txt"
    "cat test1.txt | grep \"apple\""
    "grep \"apple\" fake_file.txt"
    "grep -s \"apple\" fake_file.txt"
    "grep \"apple\" test1.txt fake_file.txt test2.txt"
    "grep \"\" test1.txt"
    "grep \"Zebra\" test1.txt"
    "grep \"[\" test1.txt"
)

SUCCESS=0
FAIL=0

for test_cmd in "${TESTS[@]}"; do
    s21_cmd="${test_cmd/grep/.\/s21_grep}"
    
    eval $test_cmd > sys_out.txt 2> /dev/null
    eval $s21_cmd > s21_out.txt 2> /dev/null
    
    diff -q sys_out.txt s21_out.txt > /dev/null
    
    if [ $? -eq 0 ]; then 
        echo "SUCCESS: $test_cmd"
        ((SUCCESS++)) 
    else 
        echo "FAIL: $test_cmd"
        ((FAIL++))
    fi 
done

echo "SUCCESS: $SUCCESS" 
echo "FAIL: $FAIL"

rm -f sys_out.txt s21_out.txt