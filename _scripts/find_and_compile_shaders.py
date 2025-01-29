#!/usr/bin/python

#  File:   find_and_compile_shaders.py
#  Author: Taylor Robbins
#  Date:   01\28\2025
#  Description: 
#  	** Finds all .glsl files in a folder, recursively, and generates (or regenerates)
#	** a matching .glsl.h file using sokol-shdc.exe.
#  Usage:
#  	** python find_and_compile_shaders.py [root_path] {--exclude="pattern"} {shdc_options...}
#	** NOTE: Any options we don't recognize will be passed along as arguments to soko-shdc.exe


import sys
import os
import subprocess
import re

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

unhandledArguments = arguments.copy();

excludePatterns = [];
targetPath = None;

def UnescapeString(strMaybeWithQuotes):
#
	result = strMaybeWithQuotes;
	if (result.startswith("\"") and result.endswith("\"")):
	#
		# TODO: This escape sequence removal strategy is flawed, we should make/use something better
		# result = result[1:-1].decode("string_escape"); #TODO: This sorta seems to work but not on some string objects that python thinks are already "decoded"
		result = result[1:-1].replace("\\\"", "\"").replace("\\'", "'").replace("\\t", "\t").replace("\\r", "\r").replace("\\n", "\n").replace("\\\\", "\\");
	#
	return result;
#
def EscapeString(str, addQuotes):
#
	# TODO: This escaping strategy is flawed, we should make/use something better
	result = str.replace("\"", "\\\"").replace("'", "\\'").replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n").replace("\\", "\\\\");
	if (addQuotes): result = "\"" + result + "\"";
	return result;
#

for arg in arguments:
#
	equalsSplitPieces = arg.split("=");
	if (len(equalsSplitPieces) == 2):
	#
		argumentName = equalsSplitPieces[0].strip();
		argumentValue = equalsSplitPieces[1].strip();
		argumentValue = UnescapeString(argumentValue);
		if (argumentName.startswith("-")): argumentName = argumentName[1:];
		if (argumentName.startswith("-")): argumentName = argumentName[1:];
		
		if (argumentName == "exclude"):
		#
			excludePatterns.append(argumentValue);
			unhandledArguments.remove(arg);
		#
		# else: print("Unknown argument name \"%s\"" % argumentName);
	#
	elif (arg.startswith("-")):
	#
		argumentName = arg;
		if (argumentName.startswith("-")): argumentName = argumentName[1:];
		if (argumentName.startswith("-")): argumentName = argumentName[1:];
		# TODO: Handle any named and non-value arguments
	#
	elif (len(equalsSplitPieces) == 1):
	#
		if (targetPath == None):
		#
			targetPath = arg.strip();
			targetPath = UnescapeString(targetPath);
			unhandledArguments.remove(arg);
		#
		else: print("Multiple path arguments given! Ignoring argument: \"%s\"" % arg);
	#
#

# NOTE: Unhandled arguments are passed to sokol-shdc.exe
# if (len(unhandledArguments) > 0):
# #
# 	print("WARNING: Unknown arguments: %s" % unhandledArguments);
# #
if (targetPath == None):
#
	print("ERROR: No target path specified!");
	print("Usage: python find_and_compile_shaders.py [root_path] {--exclude=\"pattern\"} {shdc_options...}");
	exit();
#
if (not os.path.isdir(os.fsencode(targetPath))):
#
	print("Invalid path given (the path does not exist): \"%s\"" % targetPath);
	exit();
#

# print("Target Path: \"%s\"" % targetPath);
# print("Exclude Patterns: %s" % excludePatterns);

numFilesWalked = 0;
shaderFilePaths = [];

def RecursiveWalk(pathEncoded):
#
	global numFilesWalked;
	global shaderFilePaths;
	# print("Checking \"%s\"" % os.fsdecode(pathEncoded));
	for fileEncoded in os.listdir(pathEncoded):
	#
		numFilesWalked += 1;
		filePath = os.fsdecode(fileEncoded);
		fullPath = os.fsdecode(pathEncoded) + "/" + filePath;
		if (filePath.endswith(".glsl")):
		#
			# print("Found \"%s\"" % fullPath);
			shaderFilePaths.append(fullPath);
		#
		elif (os.path.isdir(os.fsencode(fullPath))):
		#
			isExcluded = False;
			for excludePattern in excludePatterns:
			#
				if (fullPath.find(excludePattern) >= 0):
				#
					isExcluded = True;
					break;
				#
			#
			if (not isExcluded):
			#
				RecursiveWalk(os.fsencode(fullPath));
			#
		#
	#
#

RecursiveWalk(os.fsencode(targetPath));

if (len(shaderFilePaths) == 0): print("No shader files found (%d files/folders walked)" % numFilesWalked);

givenNameRegex = re.compile("Shader program: '(.*)':");

for shaderFilePath in shaderFilePaths:
#
	print("Compiling \"%s\"..." % shaderFilePath, flush=True);
	fullShaderFilePath = os.path.abspath(shaderFilePath);
	outputHeaderPath = fullShaderFilePath + ".h";
	cmd = ["sokol-shdc.exe"];
	cmd.append("--format=sokol");
	cmd.append("--errfmt=msvc");
	cmd.append("--slang=hlsl5:glsl430:metal_macos");
	cmd.append("--input=%s" % EscapeString(fullShaderFilePath, False));
	cmd.append("--output=%s" % EscapeString(outputHeaderPath, False));
	for unhandledArgument in unhandledArguments:
	#
		cmd.append(unhandledArgument);
	#
	# print("Command: %s" % " ".join(cmd));
	subprocess.check_output(cmd);
	
	shaderName = os.path.splitext(os.path.basename(shaderFilePath))[0];
	with open(outputHeaderPath, "r") as compiledShaderFile:
	#
		lineIndex = 0;
		for line in compiledShaderFile:
		#
			match = givenNameRegex.search(line);
			if (match != None):
			#
				# print("\"%s\" -> %s" % (line, match.group(1)));
				shaderName = match.group(1);
				break;
			#
			lineIndex += 1;
		#
	#
	defineName = "%s_FILE_PATH" % shaderName;
	
	with open(outputHeaderPath, "a") as compiledShaderFile:
	#
		compiledShaderFile.write("\n#define %s %s //NOTE: This line is added by find_and_compile_shaders.py\n" % (defineName, EscapeString(fullShaderFilePath, True)));
	#
	# print("Done!");
#
