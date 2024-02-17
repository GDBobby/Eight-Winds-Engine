#! bin/bash

if [ ! -d "$external" ]; then
    echo "making external"
    mkdir ./external
fi
cd external
if [ ! -d "$include" ]; then
    mkdir include
fi
cd include
if [ ! -d "$notvulkan" ]; then
    mkdir notvulkan
fi
cd notvulkan

git clone https://github.com/KhronosGroup/glslang.git
cd glslang
git checkout 30661abd9c3e97144f79cb30f2fb5a0da2c7b93c
./update_glslang_sources.py

cd ../../../../

if [ ! -d "$build" ]; then
    mkdir build
fi

cd build
cmake -S ../ -B . -DEWE_EXAMPLES=on -DEWE_PROJECTS=off
