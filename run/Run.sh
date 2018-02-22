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
dirname=`ls -d ${outputDir}/ChargingUP_Gas_${gas}_t_${thickness}_rim_${rim}_${V}V_np_${npe}_step${step}_iter${iter}`
mkdir $dirname/logout
mkdir $dirname/logerr
step_str=$step
if (( $step > 1000 )) ; then
step_str=$(( step / 1000 ))K
fi
if (( $step > 1000000 )) ; then
step_str=$(( step / 1000000 ))M
fi
iter_str=iter
if (( $iter > 1000 )) ; then
iter_str=$(( iter / 1000 ))K
fi
if (( $step > 0 )); then
file=`ls $dirname | grep charging_up_submission`
echo qsub -q N -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${step_str}${iter_str}${npe} $dirname/$file
echo qsub -q N -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${step_str}${iter_str}${npe} $dirname/$file > $dirname/SubmitLine.sh
#qsub -q N -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${step_str}${iter_str}${npe} $dirname/$file
else
echo 'Calculate the gain \(step\=0\)'
for i in $(seq 0 $iter); do
file=`ls $dirname | grep charging_up_submission_iter${i}.sh`
echo qsub -q S -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${iter_str}${npe} $dirname/$file
echo qsub -q S -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${iter_str}${npe} $dirname/$file > $dirname/SubmitLine${i}.sh
#qsub -q S -l nodes=1:ppn=$NCORES -e $dirname/logerr -o $dirname/logout -N ${gas}${V}${iter_str}${npe} $dirname/$file
done
fi



