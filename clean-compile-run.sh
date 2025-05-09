#!/usr/bin/env sh

set -e

# echo "Installing dependencies"
# sudo dnf install vulkan vulkan-headers vulkan-loader-devel \
#   glfw-devel glm-devel vulkan-validation-layers glslc clang \
#   clang-tools-extra ninja

mkdir -p assets/shaders
pushd assets/shaders
echo "Removing compiled shaders"
echo "Compiling vertex shaders"
find ./ -type f -iname "*.vert" -exec sh -c 'glslc "$1" -o "$1.spv"' _ {} \;
echo "Compiling fragment shaders"
find ./ -type f -iname "*.frag" -exec sh -c 'glslc "$1" -o "$1.spv"' _ {} \;
popd

mkdir -p build
pushd build
#rm -rf ./*
cmake -S .. -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
ninja
popd

if [ -L ./compile_commands.json ]; then
  rm ./compile_commands.json
fi
ln -s ./build/compile_commands.json ./compile_commands.json

./build/vulking
