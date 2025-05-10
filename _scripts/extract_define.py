#!/usr/bin/python

# TODO: This file is DEPRECATED in favor of C version in tools\extract_define_main.c

#  File:   extract_define.py
#  Author: Taylor Robbins
#  Date:   12\31\2024
#  Description: 
#  	** Scrapes a C/C++ header file in search for a #define with a particular name.
#  	** Outputs the value part of the #define to stdout
#  Usage:
#  	** python extract_define.py [path] [DEFINE_NAME]

import os.path
import sys
import re

# This file is script is expected to be used in a batch script like so:
# for /f "delims=" %%i in ('python ExtractDefines.py ../../../game/build_config.h DEFINE_NAME') do set DEFINE_NAME=%%i

filePath = sys.argv[1];
targetDefineName = sys.argv[2];
fileBase, fileName = os.path.split(filePath);
# print("Editing file \"" + filePath + "\"");

headerFile = open(filePath, "r");

fileContents = headerFile.read();
# print("File Contents: \"" + str(fileContents) + "\"");
headerFile.close();

searchStr = "\\#define[ \\t]+([A-Za-z_][A-Za-z0-9_]*)[ \\t]*([A-Za-z 0-9_\\t]+)";

matches = re.findall(searchStr, fileContents);
# print("Found %d #defines in %s" % (len(matches), fileName))

foundMatch = False;
for match in matches:
#
	if (match[0] == targetDefineName):
	#
		# print("Found #define %s %s" % (match[0], match[1]));
		print(match[1]);
		foundMatch = True;
		break;
	#
#

if (not foundMatch):
#
	print("Couldn't find #define %s in %s" % (targetDefineName, fileName), file=sys.stderr);
#
