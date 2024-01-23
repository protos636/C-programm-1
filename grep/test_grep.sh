#!/bin/bash

SUCCESS=0
FAIL=0

FLAGS=("-i" "-e" "-v" "-c" "-l" "-n" "-h" "-s" "-o")

PATTERNS=("sh" "l" "2" "\[a-z]" "null")

FILENAMES=("t/test_1_cat.txt" "t/test_2_cat.txt" "t/test_3_cat.txt" "t/test_4_cat.txt" "t/test_case_cat.txt")

for flag in "${FLAGS[@]}"; do
    for file in "${FILENAMES[@]}"; do
        for patterns in "${PATTERNS[@]}"; do

            sys_grep=$("grep" $flag $patterns $file)
            s21_grep=$("./s21_grep" $flag $patterns $file)

            if [ "$sys_grep" = "$s21_grep" ]; then
                ((SUCCESS++))
            else
                ((FAIL++))
                echo "./s21_grep $flag $patterns $file" >> t/logs.txt
            fi
        done
    done
done

FFLAG="t/f.txt"

for file in "${FILENAMES[@]}"; do
    for patterns in "${PATTERNS[@]}"; do

        sys_grep=$("grep" -f $FFLAG $file)
        s21_grep=$("./s21_grep" -f $FFLAG $file)

        if [ "$sys_grep" = "$s21_grep" ]; then
            ((SUCCESS++))
        else
            ((FAIL++))
            echo "./s21_grep -f $FFLAG $file" >> t/logs.txt
        fi
    done
done


echo "SUCCESS $SUCCESS"
echo "FAIL $FAIL"