# Конфігурація

CXX = g++                                    # Визначаю компілятор для C++
CC = gcc                                     # Визначаю компілятор для C
CXXFLAGS = -Wall -Wextra -std=c++11 -O2      # Налаштовую суворі прапорці та оптимізацію для C++
CFLAGS = -Wall -Wextra -std=c11 -O2          # Налаштовую прапорці та оптимізацію для мови C
LDFLAGS = -lm                                # Додаю лінкування математичної бібліотеки

# Детектування ОС
UNAME_S := $(shell uname -s)                 # Отримую назву операційної системи
ifeq ($(UNAME_S),Linux)                      # Налаштування для Linux
    RM = rm -f
    RMDIR = rm -rf
    SEP = /
    EXE =
    MKDIR = mkdir -p
endif
ifeq ($(UNAME_S),Darwin)                     # Налаштування для macOS
    RM = rm -f
    RMDIR = rm -rf
    SEP = /
    EXE =
    MKDIR = mkdir -p
endif
ifeq ($(OS),Windows_NT)                      # Налаштування для Windows
    RM = del /Q
    RMDIR = rmdir /S /Q
    SEP = \\
    EXE = .exe
    MKDIR = mkdir
endif

RM ?= rm -f                                  # Команда для видалення файлів
RMDIR ?= rm -rf                              # Команда для видалення папок
SEP ?= /                                     # Роздільник шляхів
EXE ?=                                       # Розширення виконуваних файлів
MKDIR ?= mkdir -p                            # Команда для створення папок

# Директорії

CPP_DIR = Histogram_CPP                      # Папка з файлами на С++
C_DIR = Histogram_C                          # Папка з файлами на С
BUILD_DIR = build                            # Папка для результатів збірки

# Цілі

.PHONY: all build clean run_cpp run_c cross_test help # Визначаю віртуальні цілі

# Головна ціль
all: build                                   # За замовчуванням виконую збірку всього проекту

help:                                        # Виводжу підказку по доступних командах
	@echo "Доступні команди:"
	@echo "  make build        - Скомпілювати всі програми"
	@echo "  make run_cpp      - Запустити C++ версію"
	@echo "  make run_c        - Запустити C версію"
	@echo "  make cross_test   - Крос-тест (C та C++)"
	@echo "  make clean        - Видалити скомпільовані файли"

# С++ збірка

$(BUILD_DIR)$(SEP)test_app_cpp$(EXE): $(CPP_DIR)$(SEP)Histogram.cpp $(CPP_DIR)$(SEP)Histogram_test.cpp $(CPP_DIR)$(SEP)Histogram.h
	@echo "Compiling C++..."
	@$(MKDIR) $(BUILD_DIR)               # Створюю папку для бінарних файлів
	$(CXX) $(CXXFLAGS) $(CPP_DIR)$(SEP)Histogram.cpp $(CPP_DIR)$(SEP)Histogram_test.cpp -o $(BUILD_DIR)$(SEP)test_app_cpp$(EXE) $(LDFLAGS) # Збираю С++ додаток

# С збірка

$(BUILD_DIR)$(SEP)test_app_c$(EXE): $(C_DIR)$(SEP)Histogram.c $(C_DIR)$(SEP)Histogram_test.c $(C_DIR)$(SEP)Histogram.h
	@echo "Compiling C..."
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(C_DIR)$(SEP)Histogram.c $(C_DIR)$(SEP)Histogram_test.c -o $(BUILD_DIR)$(SEP)test_app_c$(EXE) $(LDFLAGS) # Збираю С додаток

# Крос-тест

$(BUILD_DIR)$(SEP)cross_test_app$(EXE): $(C_DIR)$(SEP)Histogram.c $(CPP_DIR)$(SEP)Histogram.cpp $(CPP_DIR)$(SEP)Histogram_cross