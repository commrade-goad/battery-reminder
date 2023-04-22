#!/bin/sh
COMPILER=$1
DESTINATION=$2

print_help(){
    echo USAGE: \(1\) ./build.sh [gcc/clang]
    echo        \(2\) ./build.sh [gcc/clang] [/path/and/name]
    exit 1
}

if [ -z $DESTINATION ] 
then
    DESTINATION="./batt-reminder"
fi

echo compiling...
case $COMPILER in
    "gcc") g++ src/main.cpp -o $DESTINATION ;;
    "clang") clang++ src/main.cpp -O2 -o $DESTINATION;;
    * ) print_help;;
esac
echo saved at \`$DESTINATION\`
