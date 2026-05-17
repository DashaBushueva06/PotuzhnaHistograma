// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Заголовочний файл, що описує структуру нашого С++ класу у просторі
// імен cpp_hist. Він містить перелік усіх інструментів
// для роботи з гістограмою

#ifndef HISTOGRAM_CPP_H
#define HISTOGRAM_CPP_H

#include <cmath>
#include <string>

namespace cpp_hist {

class Histogram_cpp {
private:
    double min_hist;            // Нижня межа (лівий край графіка)
    double max_hist;            // Верхня межа (правий край графіка)
    unsigned M;                 // Кількість стовпчиків
    unsigned* frequency;        // Кількість чисел у кожному стовпчику
    unsigned long total_count;  // Облік усіх чисел, які пройшли через систему

public:
    // Етап 1: Конструктор та деструктор
    Histogram_cpp(double min_val = 0.0, double max_val = 100.0, unsigned m_val = 10); // Ініціалізую гістограму
    ~Histogram_cpp();           // Звільнюю пам'ять масиву при видаленні об'єкта

    // Етап 2: Модифікатори
    void setMax(double m);      // Змінюю праву межу діапазону
    void setMin(double m);      // Змінюю ліву межу діапазону
    void setM(unsigned m);      // Змінюю кількість стовпців

    // Етап 3: Додавання даних
    void addNumber(double x, int strategy = 1);             // Додаю по одному числу до відповідного стовпчика
    void addbatch(double data[], size_t dataSize, int strategy = 1); // Обробляю цілий масив чисел
    bool addFromConsole(int strategy);                      // Вводжу дані з консолі
    bool addFromTextFile(const char* filename, int strategy = 1);    // З текстового файлу
    bool addFromBinaryFile(const char* filename, int strategy = 1);  // З бінарного файлу
    void exportResultsToFile(const std::string& filename);  // Зберігаю розрахунки у текстовий файл-звіт

    // Етап 4: Математичні функції
    unsigned long num() const;          // Отримую загальну кількість оброблених значень N
    unsigned numHist(unsigned i) const; // Скільки значень потрапило в конкретний стовпчик i
    double mean() const;                // Вираховує вибіркове середнє арифметичне (E)
    double median() const;              // Рахую медіану
    double variance() const;            // Рахує дисперсію
    double dev() const;                 // Рахує середньоквадратичне відхилення
    double skewness() const;            // Рахує коефіцієнт асиметрії
    double kurtosis() const;            // Рахує ексцес
    double mode() const;                // Рахує моду
    double coeffVariation() const;      // Рахує відносну мінливість у відсотках

    // Етап 5: Критерії Пірсона
    double testPearsonNormal() const;      // Перевірка на нормальний розподіл
    double testPearsonUniform() const;     // Перевірка на рівномірний розподіл
    double testPearsonExponential() const; // Перевірка на експоненціальний розподіл
    double testPearsonPoisson() const;     // Перевірка на розподіл Пуассона
    double testPearsonBinomial() const;    // Перевірка на біноміальний закон

    // Утиліти та візуалізація
    double getWidth() const;               // Вираховує фізичну ширину одного стовпчика
    double getMidpoint(unsigned i) const;  // Знаходить центр i-го стовпчика
    void printHistogram() const;           // Малює графік гістограми в консолі
    void printStatistics() const;          // Пише таблицю з усіма статистичними показниками
    void printChiSquare() const;           // Виводить результати тестів Хі-квадрат

    // Допоміжні методи
    bool isValid() const;                  // Перевіряю, чи валідна гістограма
    void clear();                          // Обнуляю частоти для завантаження нових даних
};

} // namespace cpp_hist

#endif