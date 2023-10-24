#! /bin/sh

if [ $# -eq 0 ]; then
    echo "celda: usage: pass the file to be parsed as argument"
    exit 0
fi

if [ ! -e $1 ]; then
    echo "celda: error: file passed does not exist: $1"
    exit 1
fi


rows=$(grep -Pc '^\|' $1)
cells=$(grep -o '|' $1 | wc -l)

make
./celda $1 $rows $cells | column -t -s '|'
