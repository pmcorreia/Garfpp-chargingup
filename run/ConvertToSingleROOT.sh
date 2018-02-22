folder=$1
echo convert all files in `echo $folder | rev | cut -d/ -f1 | rev`
for file in `/bin/ls $folder/ChargingUpTHGEM*`;do
echo convert `echo $file | rev | cut -d/ -f1 | rev`
echo python ../scripts/Convert.py $file
python ../scripts/Convert.py $file
done

