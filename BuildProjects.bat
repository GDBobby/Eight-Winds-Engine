@echo on

mkdir external
cd external
mkdir include
cd include
mkdir notvulkan
cd notvulkan

git clone https://github.com/KhronosGroup/glslang.git
cd glslang
git checkout 30661abd9c3e97144f79cb30f2fb5a0da2c7b93c
update_glslang_sources.py

cd ../../../../

mkdir build
cd build

cmake -S ../ -B . -DEWE_PROJECTS=ON
pause