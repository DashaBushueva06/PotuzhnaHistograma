// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Файл для проведення крос-тестування, що порівнює результати роботи
// С та C++ версій Він автоматично генерує однакові набори
// даних для обох варіантів програми та зіставляє результати
// арифметичних обчислень

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

extern "C" { //Підключаю C-версію через extern "C", щоб С++ розумів назви функцій
    #include "../Histogram_C/Histogram_c.h"
    typedef Histogram CHistogram; // Використовую існуючий typedef Histogram з C-версії
}

#include "Histogram_cpp.h"

#define EPSILON_ABS 1e-10
#define EPSILON_REL 1e-6

// Функція для перевірки близькості двох чисел (враховує похибку double)
bool almostEqual(double a, double b) {
    double diff = std::abs(a - b);     // Знаходжу абсолютну різницю між двома результатами
    // Рахую динамічний поріг чутливості, який поєднує відносну та абсолютну похибки
    // Це дозволяє адекватно порівнювати як дуже великі, так і мізерні числа
    double threshold = EPSILON_REL * std::max(std::abs(a), std::abs(b)) + EPSILON_ABS;
    return diff < threshold;     // Перевіряю, чи різниця не перевищує допустиму межу епсілон
}

int main() {
    std::cout << "\n=== FINAL CROSS-TEST: C vs C++ (Full Comparison) ===\n\n";

    const int N = 5000;     // Налаштовую параметри тестування
    const double MIN_VAL = 0.0;
    const double MAX_VAL = 100.0;
    const unsigned M = 15;

    // Генерація однакових даних для обох тестів
    std::mt19937 gen(12345); // Використовую сталий seed (12345), щоб результати були відтворюваними
    std::normal_distribution<> dist(50.0, 15.0);
    std::vector<double> test_data;
    for(int i = 0; i < N; ++i) test_data.push_back(dist(gen));

    // Ініціалізація обох об'єктів
    cpp_hist::Histogram_cpp h_cpp(MIN_VAL, MAX_VAL, M); // h_cpp використовує клас із простору імен cpp_hist
    CHistogram h_c; // h_c використовує структуру та функції ініціалізації з С-версії
    initHistogram(&h_c, MIN_VAL, MAX_VAL, M);

    // Заповнення (strategy = 2 - Clamp)
    // Додаю ідентичний масив даних в обидві гістограми
    h_cpp.addbatch(test_data.data(), test_data.size(), 2);
    histAddbatch(&h_c, test_data.data(), test_data.size(), 2);

    bool all_passed = true;

    // Допоміжна лямбда-функція для красивого виведення результатів порівняння
    auto compare = [&all_passed](const char* name, double val_cpp, double val_c) {
        std::cout << std::left << std::setw(20) << name << " | "
                  << std::right << std::setw(15) << std::fixed << std::setprecision(5) << val_cpp << " | "
                  << std::right << std::setw(15) << val_c << " | ";
        if (almostEqual(val_cpp, val_c)) { // Порівнюю значення з урахуванням допустимої похибки
            std::cout << "OK\n";
        } else {
            std::cout << "MISMATCH\n";
            all_passed = false;
        }
    };

    // Формування порівняльної таблиці
    std::cout << std::left << std::setw(20) << "METRIC" << " | "
              << std::setw(15) << "CPP VERSION" << " | "
              << std::setw(15) << "C VERSION" << " | RESULT\n";
    std::cout << std::string(70, '-') << "\n";

    // Порівняння базової статистики. Зіставляю результати виклику методів класу та функцій мови С
    compare("Count", (double)h_cpp.num(), (double)histNum(&h_c));
    compare("Mean", h_cpp.mean(), histMean(&h_c));
    compare("Median", h_cpp.median(), histMedian(&h_c));
    compare("Mode", h_cpp.mode(), histMode(&h_c));
    compare("Variance", h_cpp.variance(), histVariance(&h_c));
    compare("Std Dev", h_cpp.dev(), histDev(&h_c));
    compare("Skewness", h_cpp.skewness(), histSkewness(&h_c));
    compare("Kurtosis", h_cpp.kurtosis(), histKurtosis(&h_c));
    compare("Coeff Variation %", h_cpp.coeffVariation(), variation_coeff(&h_c));

    // Порівняння критеріїв Пірсона. Перевіряю, чи однаково С та С++ рахують Хі-квадрат для різних розподілів
    std::cout << "\n--- Chi-Square Tests ---\n";
    compare("Normal", h_cpp.testPearsonNormal(), histTestPearsonNormal(&h_c));
    compare("Uniform", h_cpp.testPearsonUniform(), histTestPearsonUniform(&h_c));
    compare("Exponential", h_cpp.testPearsonExponential(), histTestPearsonExponential(&h_c));
    compare("Poisson", h_cpp.testPearsonPoisson(), histTestPearsonPoisson(&h_c));
    compare("Binomial", h_cpp.testPearsonBinomial(), histTestPearsonBinomial(&h_c, M - 1));

    std::cout << "\n" << std::string(70, '=') << "\n";
    // Виводжу фінальний статус перевірки ідентичності
    if (all_passed) {
        std::cout << ">>> FINAL STATUS: ALL CROSS-TESTS PASSED! <<<\n";
    } else {
        std::cout << ">>> FINAL STATUS: SOME TESTS FAILED! <<<\n";
    }
    std::cout << std::string(70, '=') << "\n\n";

    // Звільняю пам'ять С-структури (С++ об'єкт звільниться сам через деструктор)
    destroyHistogram(&h_c);
    return all_passed ? 0 : 1;
}