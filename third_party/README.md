# third_party Folder
This folder contains (or needs to be made to contain) copies of all third party library headers and source files. Some of these libraries are included directly in the git repository, others need to be downloaded into this folder manually before compilation. Refer to the list below to figure out which libraries are needed for which pieces of the repository, and which ones you'll need to download manually after cloning.

# List of Libraries

## stb
**URL:** [www.github.com/nothings/stb](https://www.github.com/nothings/stb)
**Folder Name:** `stb`
**Download Required:** No, all necassary files are included in the repository
**Current Version:** Commit `5c205738c191bcb0abc65c4febfa9bd25ff35234` from Nov 8th 2024
**Fixup Required:** Delete all non-essential files after cloning (everything that's not stb_*.h)
**Required For:**
* Compiling for Playdate and WebAssembly we use `stb_sprintf.h` as a printf implementation
