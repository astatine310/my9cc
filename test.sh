#!/bin/bash

./my9cc test.c > tmp.s
gcc -o tmp tmp.s
./tmp
echo "result = $?"
