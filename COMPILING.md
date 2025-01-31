# Compilation Instructions

After cloning check third_party/README.md for any libraries that need to be downloaded manually (i.e. anything that isn't included in the repository because of license/size reasons)

## On Windows for Windows
1. Make sure you have a Visual Studio installed with the MSVC compiler feature.
	a. TODO: This repository has only been tested with v19.41.34120 and uses some C23 features that are supported in that version. Using an older version of MSVC compiler is not guaranteed to work yet.
2. Locate your VS installations VsDevCmd.bat (or vcvarsall.bat for older versions of VS) and update the path in build.bat
	a. For example: `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat` or `C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat`
3. TEMP: Make sure python 3 installed as well. This is a temporary dependency that we will replace with C-based tools in the future. Python should be available as just "python" not "python3"
4. Update options in build_config.h for what you want to build (make sure BUILD_WINDOWS is 1)
5. Run build.bat

## On Windows for Linux (Using WSL)
1. If you haven't installed a Linux distro in WSL then run `wsl --install` from the command line first. This will install the default Ubuntu distro on your system
2. Enter the shell for your wsl instance by typing `wsl` into a command line
3. Run `sudo apt update`
4. We need to install clang-18 (not clang version 16 which is default from `apt install clang`).
	a. Run `wget https://apt.llvm.org/llvm.sh`
	b. Give the llvm.sh executable permission with `chmod u+x llvm.sh`
	c. Run the script with `18` as argument like `sudo ./llvm.sh 18`
	d. Verify it worked by running `clang-18 --version`
5. TODO: If compiling with Sokol or Raylib support we probably need other packages like x11-dev before compilation will work
6. TEMP: TODO: Make sure python 3 is installed as well. This is a temporary dependency that we will replace with C-based tools in the futue. Python should be available as just "python" not "python3"
7. Close the WSL shell with `exit` or just close the terminal window
8. Enable BUILD_LINUX in build_config.h (and update other defines depending on what you want to build)
9. Run build.bat (from a regular Windows batch shell, not inside WSL)

## On Linux for Linux
TODO: We haven't updated build.sh for Linux in a while! We need to get back into a non WSL linux instance and work on this
1. We need to have clang-18 installed, follow steps 3 and 4 from "On Windows for Linux (Using WSL)" above
2. Update options in build_config.h for what you want to build
3. Run build.sh

## On OSX for OSX
TODO: We haven't updated build.sh for OSX in a while! We need to get back into OSX and work on this
1. We need to have clang-18 installed, TODO: Write down steps for OSX
2. Update options in build_config.h for what you want to build
3. Run build.sh
