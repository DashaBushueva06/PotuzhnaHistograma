// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Головний файл програми для запуску та тестування С++ версії.
// Він забезпечує взаємодію з користувачем через консольне меню, дозволяючи
// обрати режим введення даних або автоматичну генерацію вибірок за
// різними законами розподілу. Файл координує процеси налаштування
// параметрів гістограми, виконання арифметичних розрахунків статистики,
// запуск перевірок гіпотез та збереження фінального звіту у файл.

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <fstream>
#include <stdexcept>
#include "Histogram_cpp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Допоміжна функція для очищення буфера вводу
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Функція для штучної генерації даних за різними законами розподілу
void generateDataCPP(const std::string& filename, bool isBinary, int distType, int count, double min, double max) {
    std::ios_base::openmode mode; // Оголошую змінну для режиму

    if (isBinary == true) {
        mode = std::ios::out | std::ios::binary; // Включає режим запису (out) та бінарний режим (binary)
    } else {
        mode = std::ios::out;
    }
    std::ofstream f;
    f.open(filename, mode); // Відкривається файл із назвою filename у вибраному режимі

    if (!f.is_open()) {
        std::cerr << "Error: Could not create file " << filename << std::endl;
        return;
    }

    // Ініціалізую генератор випадкових чисел поточним часом (time(NULL))
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    double range = max - min; //Ширина діапазону
    double mid = min + range / 2.0; //Центральна точка

    for (int i = 0; i < count; i++) {
        double val = 0;

        // Функція rand() видає ціле число від 0 до константи RAND_MAX
        // Ділю rand() на RAND_MAX, щоб отримати випадкове дробове число
        double u1 = static_cast<double>(std::rand()) / RAND_MAX;
        double u2 = static_cast<double>(std::rand()) / RAND_MAX;

        // Якщо u1 (логарифм) стане нулем -> заміна на мале число
        if (u1 <= 0) u1 = 1e-9;

        // Залежно від вибору користувача (distType), застосовую різні математичні формули
        switch (distType) {
            case 1:{
                // Нормальний розподіл Використовую перетворення Бокса-Мюллера,
                // яке з двох рівномірних чисел (u1, u2) робить одне "нормальне"
                // Множу на (range / 6.0), щоб за "правилом трьох сигм" 99% чисел влізли в межі
                // std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2) — генерує число Z ~ N(0, 1)
                double standardNormal = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
                // Масштабую Z: множу на відхилення (sigma) і додаю центр (mu)
                double sigma = range / 6.0;
                val = mid + standardNormal * sigma;
                break;
            }
            case 2:
                // Рівномірний розподіл. Беру число від 0 до 1 (u1),
                // множу на ширину діапазону і додаю мінімум
                val = min + u1 * range;
                break;
            case 3:{
                // Експоненціальний розподіл Використовую метод обернених функцій
                // Формула містить логарифм Чим ближче u1 до нуля, тим більшим вийде значення
                // -log(u1) перетворює рівномірне [0,1] в експоненціальне з лямбда = 1
                // (range / 3.0) виступає як коефіцієнт масштабу (1/lambda)
                double lambda_inv = range / 3.0;
                val = min - std::log(u1) * lambda_inv;
                break;
            }
            case 4:{
                // Розподіл Пуассона (кількість рідкісних подій)
                // Встановлюю параметр лямбда (L_val), але обмежую його числом 30,
                // бо інакше експонента exp(-L_val) стане такою малою, що округлиться
                // до нуля, і цикл стане нескінченним
                double L_val = (range / 4.0) > 30 ? 30 : (range / 4.0);
                double L_exp = std::exp(-L_val); // Граничне значення (ймовірність нуля подій)
                double p = 1.0;                  // Поточний добуток ймовірностей
                int k = 0;
                // Множу випадкові числа, поки їх добуток не стане меншим за L_exp
                // Кількість кроків (k) - число Пуассона
                do {
                    k++;
                    p *= static_cast<double>(std::rand()) / RAND_MAX;
                } while (p > L_exp);
                val = min + static_cast<double>(k - 1);
                break;
            }
            case 5:{
                // Біноміальний розподіл Генерую нормальний розподіл і округлюю
                // його до найближчого цілого числа за допомогою функції floor()
                // Використовую ту саму логіку Бокса-Мюллера, але з меншим розкидом (sigma = range/10)
                double standardNormal = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
                double continuousVal = mid + standardNormal * (range / 10.0);

                // floor(x + 0.5) — це класичний спосіб округлення до найближчого цілого в C/C++
                val = std::floor(continuousVal + 0.5);
                break;
        }
        default:
            val = min + u1 * range;
            break;
    }
        // Якщо обрано бінарний режим — записую сирі байти числа безпосередньо з пам'яті
        if (isBinary) f.write(reinterpret_cast<char*>(&val), sizeof(double));
        // Якщо текстовий — форматую число з фіксованою точністю 4 знаки після коми
        else f << std::fixed << std::setprecision(4) << val << "\n";
    }
    f.close();
}

int main() {
    int choice;
    // Показуємо меню 1 раз
    std::cout << "Select work mode (C++ Version):\n1. Console input\n2. Text file\n3. Binary file\n";

    // Локальний короткий запит Choice без повторення всього меню
    while (true) {
        std::cout << "Choice: ";
        if (std::cin >> choice && choice >= 1 && choice <= 3) {
            clearBuffer();
            break;
        }
        std::cout << "Invalid choice. Please enter 1, 2 or 3.\n";
        clearBuffer();
    }

    std::string fname = "generated_data_cpp";

    // Генерація даних
    if (choice == 2 || choice == 3 ) {
        int dist, n;

        // Локальний захист вибору розподілу
        while (true) {
            std::cout << "\nSelect distribution:\n1. Normal\n2. Uniform\n3. Exponential\n4. Poisson\n5. Binomial\n6. Random\nChoice: ";
            if (std::cin >> dist && dist >= 1 && dist <= 6) {
                clearBuffer();
                break;
            }
            std::cout << "Error: Choice is invalid. Select between 1 and 6.\n";
            clearBuffer();
        }

        // Локальний захист вводу кількості елементів
        while (true) {
            std::cout << "Enter number of elements to generate: ";
            if (std::cin >> n && n > 0) {
                clearBuffer();
                break;
            }
            std::cout << "Invalid input. Enter positive number.\n";
            clearBuffer();
        }

            fname += (choice == 2) ? ".txt" : ".dat";

        double min_h, max_h;
        int m_bins;
        while (true) {
            std::cout << "\nConfigure Histogram:\n";

            // Локальний ввід Min
            while (true) {
                std::cout << "Enter Min value: ";
                if (std::cin >> min_h) break;
                std::cout << "Invalid min. Enter number.\n";
                clearBuffer();
            }

            // Локальний ввід Max
            while (true) {
                std::cout << "Enter Max value: ";
                if (std::cin >> max_h) break;
                std::cout << "Invalid max. Enter number.\n";
                clearBuffer();
            }

            // Локальний ввід M
            while (true) {
                std::cout << "Enter number of bins (M): ";
                if (std::cin >> m_bins && m_bins > 0) {
                    clearBuffer();
                    break;
                }
                std::cout << "Invalid M. Enter positive number.\n";
                clearBuffer();
            }

            // Робимо швидку перевірку меж суто математично, щоб не висмикувати конструктор завчасно
            if (std::abs(min_h - max_h) < 1e-10) {
                std::cout << "Error: Min and Max values cannot be equal. Please enter different boundaries.\n";
                std::cout << "Let's try configuring the boundaries again.\n";
                // Повертаємося на початок циклу конфігурації
            } else {
                break; // Все супер, межі адекватні (різні), виходимо з циклу вводу
            }
        }
            // Якщо обрано роботу з файлом, генеруємо випадкові дані в межах min_h та max_h
            if (choice == 2 || choice == 3) generateDataCPP(fname, (choice == 3), dist, n, min_h, max_h);

        // Створення об'єкта класу Histogram_cpp
        cpp_hist::Histogram_cpp h(min_h, max_h, static_cast<unsigned>(m_bins));

        if (choice == 1) {
            std::cout << "\n--- Out-of-range values handling ---\n1. Discard (ignore values)\n2. Clamp (add to edge bins)\nChoice: ";
            int strategy;
            while (true) {
                std::cout << "\n--- Out-of-range values handling ---\n1. Discard (ignore values)\n2. Clamp (add to edge bins)\nChoice: ";
                if (std::cin >> strategy && (strategy == 1 || strategy == 2)) {
                    clearBuffer();
                    break;
                }
                std::cout << "Invalid choice. Please enter 1 or 2.\n";
                clearBuffer();
            }
            h.addFromConsole(strategy);
        } else if (choice == 2) {
            h.addFromTextFile(fname.c_str(), 1);
        } else {
            h.addFromBinaryFile(fname.c_str(), 1);
        }

        // Виклик методів аналізу
        h.printHistogram();
        h.printStatistics();
        h.printChiSquare();

        std::string reportName = "analysis_report_cpp.txt";
        h.exportResultsToFile(reportName);
        std::cout << "\n[OK] Analysis report saved to: " << reportName << std::endl;
    }
}