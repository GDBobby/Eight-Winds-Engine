@echo on

mkdir build
cd build
cmake -S ../ -B . -DEWE_PROJECTS=ON
pause