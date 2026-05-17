#!/bin/bash

echo "=== Building C Project ==="
cd Histogram_C
# Створюю папку build та переходжу в неї для чистої збірки
mkdir -p build && cd build
# Генерую мейкфайли через CMake та запускаю компіляцію С-версії
cmake .. && make
echo "Running C Tests..."
# Запускаю бінарний файл із тестами мови С
./histogram_c_test
cd ../..

echo ""
echo "=== Building C++ Project ==="
cd Histogram_CPP
# Створюю папку для об'єктних файлів та бінарників С++
mkdir -p build && cd build
# Конфігурую та збираю С++ версію разом із крос-тестом
cmake .. && make
echo "Running C++ Tests..."
# Запускаю головну програму на С++
./histogram_cpp_test
echo "Running Cross-Test..."
# Запускаю перевірку ідентичності результатів між С та С++
./histogram_cpp_cross
cd ../..

echo ""
echo "Done!"