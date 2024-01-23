#!/bin/bash

SUCCESS=0
FAIL=0

FLAGS=("-t" "-s" "-b" "-e" "-n")

FILENAMES=("t/test_1_cat.txt" "t/test_2_cat.txt" "t/test_3_cat.txt" "t/test_4_cat.txt" "t/test_5_cat.txt" "t/test_case_cat.txt")

for flag in "${FLAGS[@]}"; do
    for file in "${FILENAMES[@]}"; do
        sys_cat=$("cat" $flag $file)
        s21_cat=$("./s21_cat" $flag $file)

        if [ "$sys_cat" = "$s21_cat" ]; then
            ((SUCCESS++))
        else
            ((FAIL++))
            echo "./s21_cat $flag $file" >> t/logs.txt
        fi
    done
done
echo "SUCCESS $SUCCESS"
echo "FAIL $FAIL"