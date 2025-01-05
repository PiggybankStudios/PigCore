#!/bin/sh

mkdir -p _build
pushd _build
root=..

clang -I"$root" $root/piggen/piggen_main.c

popd
