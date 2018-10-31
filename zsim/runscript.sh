#! /bin/bash

for x in ./configs/*.cfg; do
    fname=${x##*/}
    echo $fname
    echo "Running " ${fname%.*}
    mkdir -p ./outputs/"${fname%.*}"
    ./build/opt/zsim $x >  ./outputs/"${fname%.*}"/"${fname%.*}".log && 2>&1
done

