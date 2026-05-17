cmake_minimum_required(VERSION 3.10)              # Встановлюю мінімально необхідну версію CMake
project(potuzhnahistogramacpp CXX C)              # Визначаю назву проекту та мови програмування

set(CMAKE_CXX_STANDARD 11)                        # Встановлюю стандарт C++11 для проекту
set(CMAKE_C_STANDARD 99)                          # Встановлюю стандарт C99 для С-частини

if(NOT MSVC)                                      # Якщо компілятор не від Microsoft
    link_libraries(m)                             # Підключаю математичну бібліотеку libm
endif()

# Шлях до кореня
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})         # Зберігаю шлях до поточної папки проекту

# ===== С та С++ версії =====
include_directories(${ROOT_DIR}/Histogram_C)      # Підключаю заголовочні файли з папки мови С
include_directories(${ROOT_DIR}/Histogram_CPP)    # Підключаю заголовочні файли С++ версії

# Збираю виконуваний файл для тестів чистої С-версії
add_executable(histogram_c_test Histogram_C/Histogram_c.c Histogram_C/Histogram_test.c)

# Збираю головний додаток С++ версії разом із С-ядром
add_executable(histogram_cpp_main Histogram_CPP/Histogram_cpp.cpp Histogram_CPP/Histogram_test.cpp
        Histogram_C/Histogram_c.c)

# Збираю програму для крос-тестування сумісності обох версій
add_executable(histogram_cpp_cross Histogram_CPP/Histogram_cpp.cpp Histogram_CPP/Histogram_cross_test.cpp Histogram_C/Histogram_c.c)