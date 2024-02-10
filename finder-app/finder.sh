#!/bin/bash

# check if the number of arguments is correct
if [ ! $# -eq 2 ]
then
  echo 'Error: Two arguments required!'
  echo 'Usage: finder.sh <filesdir> <searchstr>'
  exit 1
fi

# read arguments
filesdir=$1
searchstr=$2

# check the file directory is exist or not
if [ -d filesdir ]
then
  echo "Error: directory $filesdir not found!"
  exit 1
fi

# get files count
filesCount=$(find $filesdir -type f | wc -l)

# get matching lines count
# s - silent (no error message)
# r - recursive
# o - only matching
linesCount=$(grep -sro $searchstr $filesdir | wc -l)

# Print the result
echo "The number of files are $filesCount and the number of matching lines are $linesCount"

