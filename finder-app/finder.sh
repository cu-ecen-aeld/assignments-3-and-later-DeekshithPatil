#!/bin/sh
#Reference: https://stackoverflow.com/questions/4651437/how-do-i-set-a-variable-to-the-output-of-a-command-in-bash

if [[ $# != 2 ]] #check if the number of command line arguments are not equal to 2
then
  echo "Invalid number of command line arguments recieved!"
  exit 1
fi

if [ ! -d "$1" ] #Check if the directory passed as argument number 1 exists
then
  echo "Directory '$1' does not exist!"
  exit 1
else
  #wc is a command that performs word count on the output of a command. -l flag specifies perform "line count" only
  X=$(ls $1 | wc -l) #the number of files in the directory given by $1. NOTE: X holds the output of the command given within '()'
  Y=$(grep -r $2 $1 | wc -l) #The number of matching lines found in respective files. Text to be found is '$2' within the directory '$1'
  echo "The number of files are ${X} and the number of matching lines are ${Y}"
fi
