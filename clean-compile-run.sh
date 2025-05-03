#!/usr/bin/env sh

mkdir -p build
pushd build
rm -rf ./*
cmake ..
make
popd
./build/vulking
