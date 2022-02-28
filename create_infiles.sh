#!/bin/bash

if [ "$#" -ne 3 ]    # Check number of arguments
	then echo Please try: ./create_infiles.sh inputFile input_dir numFilesPerDirectory
	exit 1
fi

if [ ! -e $1 ]    # Check if inputFile exists
  then echo $1 does not exist
  exit 1
fi

if [ ! -f $1 ]    # Check if it is a regular file
	then echo $1 is not a regular file
	exit 1
fi

if [ ! -r $1 ]    # Check read rights
   then echo Read rights required for $1
   exit 1
fi

if [ -e $2 ]    # Check if input_dir already exists
	then echo $2 already exists!
	exit 1
fi

mkdir ./"$2";    # Create input_dir

if [ "$3" -le 0 ]    # Check requested number of files per directory
	then echo numFilesPerDirectory must be positive number
	exit 1
fi

countries=()    # Countries that have appear so far
roundRobin=()    # Indicates which file the next record should be written to

while read -r line || [[ -n "$line" ]]    # Read records from inputFile
do
	id=$(cut -d " " -f 1 <<<"$line")    # Get the id from current record
	name=$(cut -d " " -f 2 <<<"$line")    # Get the name from current record
	surname=$(cut -d " " -f 3 <<<"$line")    # Get the surname from current record
	country=$(cut -d " " -f 4 <<<"$line")    # Get the country from current record
	age=$(cut -d " " -f 5 <<<"$line")    # Get the age from current record
	virus=$(cut -d " " -f 6 <<<"$line")    # Get the virus from current record
	status=$(cut -d " " -f 7 <<<"$line")    # Get the status from current record
	
	subdirectory="./"$2"/"$country""    # Subdirectory path
	if [ ! -e "$subdirectory" ]    # Create subdirectory for this country if it doesn't exists
		then mkdir "$subdirectory";
		num=1
		while [ "$num" -le $3 ]    # Create numFilesPerDirectory
		do
			touch "$subdirectory"/"$country"-"$num".txt;
			let "num += 1"    # Increase num
		done
		countries+=("$country")    # New country encountered
		roundRobin+=(1)    # Initial file will be country-1.txt
	fi
	
	file=""
	i=0
	while true
	do
		if [ "${countries[$i]}" == "$country" ]    # Lookup country to determine its index
			then file=""$subdirectory"/"$country"-"${roundRobin[$i]}".txt"    # Based on index find the appropriate file to write the record
			let "roundRobin[$i] += 1"    # Update to take care the next record regarding this country
			if [ "${roundRobin[$i]}" -gt $3 ]    # If it exceeds numFilesPerDirectory, start all over again (Round Robin)
				then roundRobin[$i]=1
			fi
			break
		fi
		let "i += 1"    # Increase index
	done
	printf "%s " $id >> "$file"
	printf "%s " $name >> "$file"
	printf "%s " $surname >> "$file"
	printf "%s " $country >> "$file"
	printf "%s " $age >> "$file"
	printf "%s " $virus >> "$file"
	printf "%s" $status >> "$file"
	if [ "$status" == "YES" ]
		then date=$(cut -d " " -f 8 <<<"$line")    # Get the date from current record
		printf " %s" $date >> "$file"
	fi
	
	echo >> "$file"    # Line feed
done < "$1"
