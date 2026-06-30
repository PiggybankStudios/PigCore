//In order to convince clang to compile our application as Objective-C, we need to feed it a .m file.
//This file #includes tests_main.c which #includes everything else, so it basically acts the same as tests_main.c but with a .m file extension.
#include "tests_main.c"
