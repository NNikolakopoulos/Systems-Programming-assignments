#!/bin/bash


virusFile=$1;
countriesFile=$2;
numLines=$3;
duplicatesAllowed=$4;
fname="inputFile.txt";


if ! [ -f "${virusFile}" ];
then
    echo "ERROR: ${virusFile} is not a file";
    exit
fi

if ! [ -f "${countriesFile}" ];
then
    echo "ERROR: ${countriesFile} is not a file";
    exit;
fi

touch $fname;
if [ -f $fname ]
then
    rm $fname;
fi

declare -a virusArr;

declare -a countriesArr;

#count the number of lines of virus and countries files

virusFileLines=0;
while IFS= read -r l
do
    virusArr[${virusFileLines}]=${l};
    virusFileLines=$(($virusFileLines+1));
done < $virusFile;

countriesFileLines=0;
while IFS= read -r l
do
    countriesArr[${countriesFileLines}]=${l};
    countriesFileLines=$(($countriesFileLines+1));
done < $countriesFile;

# we need an assosiative array with key=citizenID   and value=firstname,lastname,age,...
declare -A arr1;
# we also need an array that has value=citizenID and key=increasing_index(1,2,3,...)
# so we can get duplicates if needed
declare -A arr2;
index=0;

# the "0-th" position of array will never be used
arr2[0]=0000;
arr1[0]="fjsdk fdjsakl Greece 10 H1N1 No";


##########################################
#create a file with "numLines" lines

i=1;
while [ ${i} -le ${numLines} ]
do
    #this loop will generate a random/duplicate citizenID
    while true
    do
        #if duplicates are allowed, there is a chance 10% to have a duplicate
        if [ $duplicatesAllowed -ne 0 ] && [ $(($RANDOM%10)) -eq 1 ] && [ ${index} -ne 0 ] 
        then
            randIndex=$((${RANDOM-1}%${index}));
            if [ $randIndex -eq 0 ]
            then
                continue;
            fi
            citizenid=${arr2[${randIndex}]};
            citizeninfo=${arr1[${citizenid}]};
            flag=0;
            break;
        else
            citizenid=$(($RANDOM%10000));  # just pick a random 4-digit number 
            if [ $citizenid -eq 0 ]
            then
                continue;
            fi
            # if this number is already in the stored array, then pick another
            if [ -z ${arr1[${citizenid}]+abc} ]
            then
                #if this id is unique, then generate the rest info for citizen
                index=$(($index+1));
                flag=1;
                break;
            else
                continue;
            fi
        fi
    done

    #if citizien id has more than 4 digits and no duplicates are allowed, error
    if [ $citizenid -gt 9999 ] && [ $duplicatesAllowed -eq 0 ]
    then
        echo "ERROR: No more unique citizenid's exists";
        exit;
    fi    

    citizenIDint=$citizenid
    if [ $citizenIDint -lt 10 ]
    then 
        citizenid="0${citizenid}";
    fi
    if [ $citizenIDint -lt 100 ]
    then 
        citizenid="0${citizenid}";
    fi
    if [ $citizenIDint -lt 1000 ]
    then 
        citizenid="0${citizenid}";
    fi

    if [ $flag -eq 1 ]
    then
        
        #age is a random number<=120
        age=$(($RANDOM%121));

        letters=AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz;
        firstname="";
        #max characters of first name are at most 12
        upperbound=$(($RANDOM%10+3));
        j=1
        while [ $j -le $upperbound ]
        do
            # add a random character to first name, until we have the desired length
            firstname="${firstname}${letters:RANDOM%${#letters}:1}"
            j=$(($j+1));
        done
        lastname="";
        upperbound=$(($RANDOM%10+3));
        j=1
        while [ $j -le $upperbound ]
        do
            lastname="${lastname}${letters:RANDOM%${#letters}:1}"
            j=$(($j+1));
        done

    

    ########################get a random country

        country=$((${RANDOM}%${countriesFileLines}));
        countryname=${countriesArr[${country}]};
        #j=0;
       # while IFS= read -r line 
        #do
       #     if [ ${country} -eq $j ]
        #    then
        #        countryname=$line;
        #        break;
        #    fi
        #    j=$(($j+1));
       # done < $countriesFile;

    ###################### save citizen info in arrays for duplicates later use #######################
        citizeninfo="${firstname} ${lastname} ${countryname} ${age}";
        arr1[${citizenIDint}]=$citizeninfo;
        arr2[${index}]=$citizenIDint;
    fi
    ############################# get a random virus

        virus=$((${RANDOM} % ${virusFileLines}));
        virusname=${virusArr[${virus}]};
        #j=0;
        #while IFS= read -r line 
       # do
        #    if [ ${virus} -eq $j ]
        #    then
        #        virusname=$line;
        #        break;
        #    fi
        #        j=$(($j+1));
       # done < $virusFile;

    ########################### get a random date now

    yearoffset=$(($RANDOM%4))
    year=$(($yearoffset+2018))
    month=$(($RANDOM%13))
    day=$(($RANDOM%32))

    if [ $month -lt 10 ]
    then 
        month="0${month}";
    fi
    if [ $day -lt 10 ]
    then
        day="0${day}";
    fi

    date="${day}-${month}-${year}";

    if [ $(($RANDOM%3)) -eq 1 ]
    then
        vaccinated="YES";
        echo "${citizenid} ${citizeninfo} ${virusname} ${vaccinated} ${date}"  >> $fname
    else
        vaccinated="NO";
        echo "${citizenid} ${citizeninfo} ${virusname} ${vaccinated}"  >> $fname
    fi

    i=$((${i}+1));
done


