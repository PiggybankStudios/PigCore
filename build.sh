#!/bin/bash

# TODO: We should pull out PIG_BUILD_RELATIVE_PATH and PIG_BUILD_ABSOLUTE_PATH like the batch script
if [ ! -d "../pig_build" ]; then
	if [ ! which git ]; then
		echo "Git is not installed! Please download PigBuild into $(cd "../pig_build" && pwd)"
	fi
	git clone https://github.com/PiggybankStudios/PigBuild ../pig_build
fi

../pig_build/shell/build.sh $@