# Pigglen
Pigglen is a rewrite of pieces of other repositories I've created in the past like [Pig Engine](https://github.com/PiggybankStudios/PigEngine), [GyLib](https://github.com/PiggybankStudios/GyLib), etc. but written in pure C (not C++) and with some new goals and assumptions that align better with my goals as a programmer. The name derives from Pig Engine (Pig is a useful prefix for names in the codebase since it's 3 letters and easy to pronounce) and the english word "glen", implying this is a pleasent space that has good code

## Goals
1. Keep documentation up-to-date as things are built. We don't need documentation for everything but we should always have a minimum bar of explanation written down for things like folder structure and namespaces
2. Maintain a pure C codebase, get back to feature parity with things from Pig Engine and GyLib without using any C++ features
3. Keep It Simple. Try to do more with less code, focus on the things that are actually proven useful, and separate those from things that are experimental. Actively remove (or at least move) code that proves useless or minimally useful
4. Split GyLib into more usable parts. OS level stuff can be used in `mem/` and `struct/`, not just a "platform layer" like Pig Engine had. Macros and things in `base/` can be used without using the entire library (depending upon subsets of GyLib was always a fraught)
5. Revisit and extend or refine these goals as I go along. Having the goals written down helps us focus on them more concretely

## Language/Compiler Choice
Every source file in this repository is compatible with the C23 language spec and is able to be compiled by MSVC, Clang, and GCC on Windows, OSX, and Linux. Some files may comply to an earlier C language spec and could be used by various projects that want to target something like C99 but that isn't a guarantee for all files here. If a file is C99 compliant than all of it's dependencies are also C99 complient or earlier. Note that while we do maintain the ability to compile with multiple compilers and on multiple platforms, much of the functionality in the codebase may be stubbed out for platforms where we aren't actively developing. These stubs will get filled out on an as-needed basis so we don't spend so much time setting up for platforms that we aren't shipping a particular piece of software on.

## Folder Structure
Each folder is going to have a "category" that describes the kind of code exists there, a README.md can be found in each folder describing what that category is and why the name makes sense. Note that the folders aren't extremely tied to their dependency order. For example we may have one file in `os/` that is depended upon by some of the lowest levels of the codebase, and therefore can't use higher level constructure like Arena or Strings. But we might also have a nother file in `os/` that depends upon data structures from `struct/`. These two files may even have related code, like stuff for dealing with files, but by splitting them between two files we are able to use the more fundamental things defined in the first file from more places and optionally include the second file when we actually want the extra functionality that brings in more dependencies.

## License
This repository is licensed under the MIT License, see the LICENSE.txt file for more information.