#!/bin/bash

if [[ $# != 2 ]] #check if the number of command line arguments are not equal to 2
then
  echo "Invalid number of command line arguments recieved!"
  exit 1
fi

output=$(echo $2 > $1) #$2 holds the text to be written, $1 holds the file path

if [ $? -ne 0 ] #Check if the above operation was unsuccessfull
then
  echo "Exiting with value 1"
  exit 1
fi
