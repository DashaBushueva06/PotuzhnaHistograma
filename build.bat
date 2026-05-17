@echo off
:: Вимикаю вивід самих команд, щоб у консолі було видно лише результат
echo === Building C Project ===

cd Histogram_C
:: Перевіряю наявність папки build та створюю її за потреби
if not exist build mkdir build
cd build

:: Генерую проектні файли для Visual Studio 16 2019 через CMake
cmake .. -G "Visual Studio 16 2019"
:: Запускаю компіляцію С-версії у режимі Release
cmake --build . --config Release

echo Running C Tests...
:: Запускаю скомпільований файл тестів мови С
.\Release\histogram_c_test.exe
cd ../..

echo.
echo === Building C++ Project ===

cd Histogram_CPP
:: Створюю окрему папку build для С++ версії
if not exist build mkdir build
cd build

:: Конфігурую проект С++ через CMake
cmake .. -G "Visual Studio 16 2019"
:: Виконую збірку С++ коду та крос-тесту
cmake --build . --config Release

echo Running C++ Main and Cross-Test...
:: Запускаю головний тест С++ версії
.\Release\histogram_cpp_test.exe
:: Запускаю порівняльний крос-тест між С та С++
.\Release\histogram_cpp_cross.exe
cd ../..

echo.
echo Done!
:: Ставлю паузу, щоб вікно консолі не закрилося одразу після роботи
pause