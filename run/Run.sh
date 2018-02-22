#!/bin/zsh
if [ "$#" -ne 1 ]; then
  echo "Error, no input file, Usage: $0 RunList.txt" >&2
  exit 1
fi

outputDir=/tmp/$USERNAME

line=`cat $1 | head -2 | tail -1`
gas=`echo $line | cut -d' ' -f1`
thickness=`echo $line | cut -d' ' -f2`
rim=`echo $line | cut -d' ' -f3`
V=`echo $line | cut -d' ' -f4`
npe=`echo $line | cut -d' ' -f5`
step=`echo $line | cut -d' ' -f6`
iter=`echo $line | cut -d' ' -f7`
NCORES=`echo $line | cut -d' ' -f8`


echo python ../scripts/charging_up_submission_parallel.py $thickness $rim $V $step $iter $npe $gas $NCORES
python ../scripts/charging_up_submission_parallel.py $thickness $rim $V $step $iter $npe $gas $NCORES
