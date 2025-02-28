#!/bin/bash

uname_output=`uname -a`
if [[ "$uname_output" =~ Linux ]] then
	./build_linux.sh
elif [[ "$uname_output" =~ OSX ]] then
	./build_osx.sh
else
	echo "We can't decide whether you are running on Linux or OSX based off the output of uname -a: \"$uname_output\""
fi
