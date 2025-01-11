#!/usr/bin/python

#  File:   combine_files.py
#  Author: Taylor Robbins
#  Date:   01\10\2025
#  Description: 
#  	** Combines 2 or more files into a single one, adding a single new-line in-between each file contents
#  Usage:
#  	** python combine_files.py [output_path] [file1] [file2] ... [filen]

import sys

def plural(count):
#
	if (count == 1): return "";
	else: return "s";
#

numArguments = len(sys.argv);
arguments = sys.argv;

# First argument is always the path to the python file
if (numArguments > 1):
#
	numArguments -= 1;
	arguments = arguments[1:];
#

# print("Number of arguments %s" % numArguments);
# print("Arguments %s" % (str(arguments)));

if (numArguments < 2):
#
	print("Not enough arguments passed. Expected at least 2 more arguments (one for output path and at least one input path)");
	exit();
#

outputPath = arguments[0];
inputPaths = arguments[1:];

print("Combining %s file%s into \"%s\"" % (len(inputPaths), plural(len(inputPaths)), outputPath));

outputContents = "";

for inputPath in inputPaths:
#
	with open(inputPath) as inputFile:
	#
		outputContents += "\n" + inputFile.read();
	#
#

with open(outputPath, "w") as outputFile:
#
	outputFile.write(outputContents);
#

print("Finished! (%d characters total)" % (len(outputContents)));
