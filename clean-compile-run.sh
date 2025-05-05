#!/usr/bin/env sh

set -e

mkdir -p assets/shaders
pushd assets/shaders
echo "Removing compiled shaders"
# sudo dnf install glslc
echo "Compiling vertex shaders"
find ./ -type f -iname "*.vert" -exec sh -c 'glslc "$1" -o "$1.spv"' _ {} \;
echo "Compiling fragment shaders"
find ./ -type f -iname "*.frag" -exec sh -c 'glslc "$1" -o "$1.spv"' _ {} \;
popd

mkdir -p build
pushd build
rm -rf ./*
cmake -S .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make
popd

rm ./compile_commands.json
ln -s ./build/compile_commands.json ./compile_commands.json

./build/vulking
