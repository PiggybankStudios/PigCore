# PigCore
PigCore is a rewrite of pieces of other repositories I've created in the past like [Pig Engine](https://github.com/PiggybankStudios/PigEngine), [GyLib](https://github.com/PiggybankStudios/GyLib), etc. but written in pure C (not C++) and with some new goals and assumptions that align better with my goals as a programmer. The name "Pig" is a useful prefix for names in the codebase since it's 3 letters and easy to pronounce, "Core" simply describes that this repository contains all the of the core functionality of a project that depends on Pig technology.

## Goals
1. Keep documentation up-to-date as things are built. We don't need documentation for everything but we should always have a minimum bar of explanation written down for things like folder structure and namespaces
2. Maintain a pure C codebase, get back to feature parity with things from Pig Engine and GyLib without using any C++ features
3. Keep It Simple. Try to do more with less code, focus on the things that are actually proven useful, and separate those from things that are experimental. Actively remove (or at least move) code that proves useless or minimally useful
4. Split GyLib into more usable parts. OS level stuff can be used in `mem/` and `struct/`, not just a "platform layer" like Pig Engine had. Macros and things in `base/` can be used without using the entire library (depending upon subsets of GyLib was always a bit fraught)
5. Revisit and extend or refine these goals as I go along. Having the goals written down helps us focus on them more concretely
6. Embrace code generation as a way to overcome specific language deficiencies (like getting enum name strings automatically, or setting up serialization for a type with only markup on the struct)
7. Make a codebase that is helpful and usable for "complete" and "production ready" software. We aren't trying to make the cleanest or simplest set of features, we are trying to make something that has ALL the features necassary for a product to function well.
8. Maintain a small degree of backwards compatability. Since this codebase is going to be used by multiple projects we may make changes while working on a newer project that break existing projects. We should consider if there is a clean and reasonable way to keep the old projects compiling first. However, if that's not possible we should make sure to note the breaking change clearly in a comment and/or in the commit logs so that when we return to the old project we are able to decipher what changes are required to bring the project "up to date" with Pig Core and working again. The recommended process for fixing up code after a breaking change should always be clear! We should very rarely (if ever) make changes where the new code does not support the functionality of the old code, unless we are sure that all existing projects don't want/need the old functionality.

## Areas of Interest
There are a few categories of things that I want to explore that I have not been able to before with PigEngine because of the design decisions made throughout that codebase. As such these are new targets and design goals that will affect how this codebase is organized compared to prior codebases:
1. WebAssembly as a shipping platform for basic tools and games, likely using WebGL or WebGPU
2. Experimental VR games using OpenVR
3. Native Android and iOS app development
4. Bare-bone graphical prototypes with support for swift iteration
5. CLI tools that help us manipulate or generate files
6. Mixed language projects (for example C and Jai living in the same codebase and interopability between the too)
7. Cross-platform and multi-compiler support
8. Game related tooling in a separate executable from the engine/game
9. Single executable shipping format (resources, shaders, etc. all baked in)

## Language/Compiler Choice
Every source file in this repository is compatible with the C23 language spec and is able to be compiled by MSVC, Clang, and GCC on Windows, OSX, and Linux. Some files may comply to an earlier C language spec and could be used by various projects that want to target something like C99 but that isn't a guarantee for all files here. If a file is C99 compliant than all of it's dependencies are also C99 complient or earlier. Note that while we do maintain the ability to compile with multiple compilers and on multiple platforms, much of the functionality in the codebase may be stubbed out for platforms where we aren't actively developing. These stubs will get filled out on an as-needed basis so we don't spend so much time setting up for platforms that we aren't shipping a particular piece of software on.

## Folder Structure
Each folder is going to have a "category" that describes the kind of code exists there, a README.md can be found in each folder describing what that category is and why the name makes sense. Note that the folders aren't extremely tied to their dependency order. For example we may have one file in `os/` that is depended upon by some of the lowest levels of the codebase, and therefore can't use higher level constructure like Arena or Strings. But we might also have a nother file in `os/` that depends upon data structures from `struct/`. These two files may even have related code, like stuff for dealing with files, but by splitting them between two files we are able to use the more fundamental things defined in the first file from more places and optionally include the second file when we actually want the extra functionality that brings in more dependencies.

## License
This repository is licensed under the MIT License, see the LICENSE.txt file for more information.