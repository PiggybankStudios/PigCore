# `third_party/_tools` Folder
This folder contains third_party compiled binaries that help us build PigCore (or PigCore projects). For example it contains the sokol-shdc CLI tool that allows us to cross-compile our graphics shader code and use it easily with Sokol.

# List of Tools

## Sokol Tools (SHDC)

**URL:** [Binaries](https://github.com/floooh/sokol-tools-bin) [Source Code](https://github.com/floooh/sokol-tools/tree/master)

**Platforms:** `win32`, `linux`, `linux_arm`, `osx`, `osx_arm`

**Download Required:** No, all necassary files are included in the repository

**Current Version:** Commit `03138cef005bc75ef047998a8784b93360486d00` from Feb 23rd 2026. [Commit Link](https://github.com/floooh/sokol-tools-bin/commit/03138cef005bc75ef047998a8784b93360486d00)

**Binary Name:** `sokol-shdc(.exe)`

**Fixup Required:** Copy platform specific binaries to each folder. Remove readme.txt files from `win32`, `linux` and `osx` folders. Run `win32\sokol-shdc.exe --help 2> sokol_shdc_help.txt`

## Protobuf-C (protoc-gen-c)

==TODO:== Fill out information for this

**Platforms:** `linux`
