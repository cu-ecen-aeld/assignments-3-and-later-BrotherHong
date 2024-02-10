#!/bin/bash

# check the number of arguments
if [ ! $# -eq 2 ]
then
  echo 'Error: Two arguments is required!'
  exit 1
fi

# read arguments
writefile=$1
writestr=$2

# make directory
mkdir -p $(dirname $writefile)

# write file
echo $writestr > $writefile

