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
listOutputPath = None;

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
		elif (argumentName == "list_file"):
		#
			listOutputPath = argumentValue;
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

absTargetPath = os.path.abspath(targetPath)
RecursiveWalk(os.fsencode(targetPath));

if (len(shaderFilePaths) == 0): print("No shader files found (%d files/folders walked)" % numFilesWalked);

noEntriesString = "no_entries";
givenNameRegex = re.compile("Shader program: '([A-Za-z_][A-Za-z0-9_]*)':"); # Shader program: 'main2d':
imageRegex = re.compile("Image '([A-Za-z_][A-Za-z0-9_]*)':"); # Image 'main2d_texture0':
samplerRegex = re.compile("Sampler '([A-Za-z_][A-Za-z0-9_]*)':"); # Sampler 'main2d_sampler0':
allShaderNames = [];
allShaderSourcePaths = [];

for shaderFilePath in shaderFilePaths:
#
	print("Compiling \"%s\"..." % shaderFilePath, flush=True);
	fullShaderFilePath = os.path.abspath(shaderFilePath);
	outputHeaderPath = fullShaderFilePath + ".h";
	outputSourcePath = fullShaderFilePath + ".c";
	cmd = ["sokol-shdc.exe"];
	cmd.append("--format=sokol_impl");
	cmd.append("--errfmt=msvc");
	# cmd.append("--reflection");
	cmd.append("--slang=hlsl5:glsl430:metal_macos");
	cmd.append("--input=%s" % EscapeString(fullShaderFilePath, False));
	cmd.append("--output=%s" % EscapeString(outputHeaderPath, False));
	for unhandledArgument in unhandledArguments:
	#
		cmd.append(unhandledArgument);
	#
	# print("Command: %s" % " ".join(cmd));
	subprocess.check_output(cmd);
	allShaderSourcePaths.append(outputSourcePath);
	
	shaderName = "unknown";
	foundName = False;
	shaderImages = [];
	shaderSamplers = [];
	uniformBlocks = [];
	shaderUniforms = [];
	insideUniformBlock = None;
	shaderAttributes = [];
	attributeRegex = None;
	uniformBlockRegex = None;
	
	with open(outputHeaderPath, "r") as compiledShaderFile:
	#
		lineIndex = 0;
		for line in compiledShaderFile:
		#
			if (not foundName):
			#
				nameMatch = givenNameRegex.search(line);
				if (nameMatch != None):
				#
					# print("\"%s\" -> %s" % (line, nameMatch.group(1)));
					shaderName = nameMatch.group(1);
					attributeRegex = re.compile("\\#define\\s+ATTR_%s_(.*)\\s+\\(\\d+\\)" % shaderName);
					uniformBlockRegex = re.compile("^\\s*C struct\\:\\s+(.*)_t\\s*$");
					foundName = True;
				#
			#
			else:
			#
				imageMatch = imageRegex.search(line);
				if (imageMatch != None):
				#
					# print("\"%s\" -> %s" % (line, imageMatch.group(1)));
					shaderImages.append(imageMatch.group(1));
				#
				samplerMatch = samplerRegex.search(line);
				if (samplerMatch != None):
				#
					# print("\"%s\" -> %s" % (line, samplerMatch.group(1)));
					shaderSamplers.append(samplerMatch.group(1));
				#
				attribMatch = attributeRegex.search(line);
				if (attribMatch != None):
				#
					# print("\"%s\" -> %s" % (line, attribMatch.group(1)));
					shaderAttributes.append(attribMatch.group(1));
				#
				uniformBlockMatch = uniformBlockRegex.search(line);
				if (uniformBlockMatch != None):
				#
					# print("\"%s\" -> %s" % (line, uniformBlockMatch.group(1)));
					uniformBlocks.append(uniformBlockMatch.group(1));
				#
				if (insideUniformBlock == None):
				#
					for uniformBlock in uniformBlocks:
					#
						uniformBlockStartRegex = re.compile("typedef\\s*struct\\s*%s_t\\s*\\{" % uniformBlock);
						if (uniformBlockStartRegex.search(line) != None):
						#
							# print("\"%s\" -> %s" % (line, uniformBlockStartMatch.group(1)));
							insideUniformBlock = uniformBlock;
						#
					#
				#
				else:
				#
					uniformBlockEndRegex = re.compile("}\\s*%s_t;" % insideUniformBlock);
					uniformRegex = re.compile("^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s+([A-Za-z_][A-Za-z0-9_]*);\\s*$");
					uniformMatch = uniformRegex.search(line);
					if (uniformBlockEndRegex.search(line) != None):
					#
						insideUniformBlock = None;
					#
					elif (uniformMatch != None):
					#
						# print("\"%s\" -> block=%s type=%s name=%s" % (line, insideUniformBlock, uniformMatch.group(1), uniformMatch.group(2)));
						uniformTuple = ( insideUniformBlock, uniformMatch.group(1), uniformMatch.group(2) );
						shaderUniforms.append(uniformTuple);
					#
				#
			#
			lineIndex += 1;
		#
	#
	
	numImages = len(shaderImages);
	numSamplers = len(shaderSamplers);
	numUniforms = len(shaderUniforms);
	numAttributes = len(shaderAttributes);
	if (len(shaderImages) == 0): shaderImages.append(noEntriesString);
	if (len(shaderSamplers) == 0): shaderSamplers.append(noEntriesString);
	if (len(shaderAttributes) == 0): shaderAttributes.append(noEntriesString);
	if (len(shaderUniforms) == 0): shaderUniforms.append((noEntriesString, "void", noEntriesString));
	
	if (foundName):
	#
		allShaderNames.append(shaderName);
		filePathDefineName = "%s_SHADER_FILE_PATH" % shaderName;
		imageCountDefineName = "%s_SHADER_IMAGE_COUNT" % shaderName;
		imageDefsDefineName = "%s_SHADER_IMAGE_DEFS" % shaderName;
		samplerCountDefineName = "%s_SHADER_SAMPLER_COUNT" % shaderName;
		samplerDefsDefineName = "%s_SHADER_SAMPLER_DEFS" % shaderName;
		uniformCountDefineName = "%s_SHADER_UNIFORM_COUNT" % shaderName;
		uniformDefsDefineName = "%s_SHADER_UNIFORM_DEFS" % shaderName;
		attributeCountDefineName = "%s_SHADER_ATTR_COUNT" % shaderName;
		attributeDefsDefineName = "%s_SHADER_ATTR_DEFS" % shaderName;
		with open(outputHeaderPath, "a") as compiledShaderFile:
		#
			imageDefs = "{ \\\n";
			for imageName in shaderImages:
			#
				if (imageName == noEntriesString): imageDefs += "\t{ .name=NO_ENTRIES_STR, .index=0 }, \\\n";
				else: imageDefs += "\t{ .name=\"%s\", .index=IMG_%s }, \\\n" % (imageName, imageName);
			#
			imageDefs += "} // These should match ShaderImageDef struct found in gfx_shader.h";
			
			samplerDefs = "{ \\\n";
			for samplerName in shaderSamplers:
			#
				if (samplerName == noEntriesString): samplerDefs += "\t{ .name=NO_ENTRIES_STR, .index=0 }, \\\n";
				else: samplerDefs += "\t{ .name=\"%s\", .index=SMP_%s }, \\\n" % (samplerName, samplerName);
			#
			samplerDefs += "} // These should match ShaderSamplerDef struct found in gfx_shader.h";
			
			uniformDefs = "{ \\\n";
			for uniformTuple in shaderUniforms:
			#
				uniformBlock = uniformTuple[0]; uniformType = uniformTuple[1]; uniformName = uniformTuple[2];
				if (uniformName == noEntriesString): uniformDefs += "\t{ .name=NO_ENTRIES_STR, .blockIndex=0, .offset=0, .size=0 }, \\\n";
				else: uniformDefs += "\t{ .name=\"%s\", .blockIndex=UB_%s, .offset=STRUCT_VAR_OFFSET(%s_t, %s), .size=STRUCT_VAR_SIZE(%s_t, %s) }, \\\n" % (uniformName, uniformBlock, uniformBlock, uniformName, uniformBlock, uniformName);
			#
			uniformDefs += "} // These should match ShaderUniformDef struct found in gfx_shader.h";
			
			attributeDefs = "{ \\\n";
			for attribute in shaderAttributes:
			#
				if (attribute == noEntriesString): attributeDefs += "\t{ .name=NO_ENTRIES_STR, .index=0 }, \\\n";
				else: attributeDefs += "\t{ .name=\"%s\", .index=ATTR_%s_%s }, \\\n" % (attribute, shaderName, attribute);
			#
			attributeDefs += "} // These should match ShaderAttributeDef struct found in gfx_shader.h";
			
			compiledShaderFile.write("\n\n//NOTE: These lines were added by find_and_compile_shaders.py\n");
			compiledShaderFile.write("//NOTE: Because an empty array is invalid in C, we always add at least one dummy entry to these definition #defines while the corresponding COUNT #define will remain 0\n");
			compiledShaderFile.write("#ifndef NO_ENTRIES_STR\n");
			compiledShaderFile.write("#define NO_ENTRIES_STR \"%s\"\n" % noEntriesString);
			compiledShaderFile.write("#endif\n");
			compiledShaderFile.write("#define %s %s\n" % (filePathDefineName, EscapeString(fullShaderFilePath, True)));
			compiledShaderFile.write("#define %s %s\n" % (imageCountDefineName, numImages));
			compiledShaderFile.write("#define %s %s\n" % (imageDefsDefineName, imageDefs));
			compiledShaderFile.write("#define %s %s\n" % (samplerCountDefineName, numSamplers));
			compiledShaderFile.write("#define %s %s\n" % (samplerDefsDefineName, samplerDefs));
			compiledShaderFile.write("#define %s %s\n" % (uniformCountDefineName, numUniforms));
			compiledShaderFile.write("#define %s %s\n" % (uniformDefsDefineName, uniformDefs));
			compiledShaderFile.write("#define %s %s\n" % (attributeCountDefineName, numAttributes));
			compiledShaderFile.write("#define %s %s\n" % (attributeDefsDefineName, attributeDefs));
		#
	#
	else:
	#
		print("WARNING: Failed to find given name in generated shader header file. This file will not have the tacked-on defines that we expect! %s" % (outputHeaderPath));
	#
	
	headerIncludePath = outputHeaderPath.replace("\\", "/").split("/")[-1];
	with open(outputSourcePath, "w") as shaderSourceFile:
	#
		shaderSourceFile.write("\n");
		shaderSourceFile.write("#include \"shader_include.h\"\n");
		shaderSourceFile.write("\n");
		shaderSourceFile.write("#include \"%s\"\n" % headerIncludePath);
	#
#

if (listOutputPath != None):
#
	with open(listOutputPath, "w") as listFile:
	#
		headerIndex = 0;
		for headerPath in allShaderSourcePaths:
		#
			if (headerIndex > 0): listFile.write(",");
			listFile.write("%s" % os.path.relpath(headerPath));
			headerIndex += 1;
		#
	#
#
