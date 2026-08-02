#!/bin/bash

PIG_BUILD_RELATIVE_PATH="../pig_build"
PIG_BUILD_ABSOLUTE_PATH="$(cd .. && pwd)/pig_build"
if [ ! -d $PIG_BUILD_RELATIVE_PATH ]; then
	if [ ! $(which git) ]; then
		echo "Git is not installed! Please manually download PigBuild into $PIG_BUILD_ABSOLUTE_PATH"
		exit
	fi
	git clone https://github.com/PiggybankStudios/PigBuild $PIG_BUILD_RELATIVE_PATH
fi

$PIG_BUILD_RELATIVE_PATH/shell/build.sh $@