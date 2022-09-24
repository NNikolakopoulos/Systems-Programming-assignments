#!/bin/bash

inputFile=$1;
input_dir=$2;
numFilesPerDirectory=$3;

# some error handling

if [ $# -ne 3 ];
then
    echo "ERROR: 3 command line arguments are required";
    exit;
fi
# if this directory exists , then exit
if [ -d "${input_dir}" ];
then
    echo "ERROR: ${input_dir} already exists, exiting...";
    exit;
else
    mkdir "${input_dir}"
fi

if ! [ -f "${inputFile}" ];
then
    echo "ERROR: ${inputFile} does not exists";
    exit;
fi



######################################
# create an array with every country #
declare -a countryArr;
# create a set to check if a country already is inserted into the array. Also, it will be used for round robin distribution of records into each country file
declare -A countrySet

i=0;
while IFS= read -r l
do
    record=($l)
    if ! [ ${countrySet[${record[3]}]} ];
    then
        countryArr[${i}]=${record[3]};
        countrySet[${record[3]}]=1;
        i=$(($i+1));
    fi
done < $inputFile;
countryArrSize=${i}


# now create sub-directories

for (( i=0; i<${countryArrSize}; i++ )) # for every country stored in the previous array
do
    country="${countryArr[${i}]}"
    oldpath="${input_dir}/${country}"
    mkdir $oldpath
    for (( j=1; j<=${numFilesPerDirectory}; j++ ))
    do 
        newpath="${oldpath}/${country}-${j}.txt"
        touch $newpath
    done
done


# now fill the files of each country, with the records from input file
while IFS= read -r l
do
    record=($l);
    if [ ${countrySet[${record[3]}]} -eq $(($numFilesPerDirectory+1)) ];
    then
        countrySet[${record[3]}]=1;
        i=$(($i+1));
    fi

    newpath="${input_dir}/${record[3]}/${record[3]}-${countrySet[${record[3]}]}.txt";
    echo $l >> $newpath;
    countrySet[${record[3]}]=$((${countrySet[${record[3]}]}+1)); # increase round robin index for this country
done < $inputFile;



# If empty files exists, then delete them

for (( i=0; i<${countryArrSize}; i++ )) # for every country
do
    country="${countryArr[${i}]}"
    oldpath="${input_dir}/${country}"
    for (( j=1; j<=${numFilesPerDirectory}; j++ ))
    do 
        newpath="${oldpath}/${country}-${j}.txt"
        if [ -s ${newpath} ]  # if its not empty continue
        then
            continue;
        else
            rm ${newpath};
        fi
    done
done