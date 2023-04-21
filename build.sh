#!/bin/sh
COMPILER=$1

print_help(){
    echo USAGE: ./build.sh [gcc/clang]
    exit 1
}

case $COMPILER in
    "gcc") g++ src/main.cpp -o batt-reminder -O2;;
    "clang") clang++ src/main.cpp -o batt-reminder -O2;;
    * ) print_help;;
esac
