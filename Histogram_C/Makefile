# Визначаю компілятори для C++ та C
CXX = g++
CC = gcc

# Встановлює прапорці компіляції для суворої перевірки помилок та підтримки стандартів
CXXFLAGS = -Wall -Wextra -std=c++17
CFLAGS = -Wall -Wextra -std=c11

# Додає прапорець лінкування math.h
LDFLAGS = -lm

# За замовчуванням виконує збірку та запуск тестів
all: build run

# Збирає C++ програму з файлів реалізації та тестів
build:
	$(CXX) $(CXXFLAGS) Histogram.cpp Histogram_test.cpp -o test_app_cpp

# Запускає скомпільований бінарний файл C++ тестів
run: build
	./test_app_cpp

# Виконує збірку та запуск крос-тестів для перевірки сумісності C та C++ коду
cross_test:
	@echo "Збірка крос-тесту..."
	# Компілюю С-файл в об'єктний файл
	$(CC) $(CFLAGS) -c ../Histogram_C/Histogram.c -o Histogram_c.o
	# Компілюю C++ файл в об'єктний файл
	$(CXX) $(CXXFLAGS) -c Histogram.cpp -o Histogram_cpp.o
	# Лінкую обидва об'єктні файли з крос-тестом у єдиний додаток
	$(CXX) $(CXXFLAGS) Histogram_cross_test.cpp Histogram_c.o Histogram_cpp.o -o cross_test_app $(LDFLAGS)
	@echo "Запуск крос-тесту:"
	./cross_test_app

# Очищає проект: видаляю виконувані файли, об'єктні модулі та звіти
clean:
	rm -f test_app_cpp cross_test_app *.o Histogram_results.txt