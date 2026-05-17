// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Заголовочний файл, що визначає структуру Histogram та інтерфейс
// для керування статистичними даними. Він містить оголошення функцій
// для ініціалізації об'єкта, зміни його параметрів та очищення пам'яті.
// У файлі представлені прототипи для зчитування даних із консолі, текстових і
// бінарних файлів, а також методи для проведення арифметичних розрахунків
// статистичних показників і перевірки п'яти критеріїв Пірсона. Крім того,
// задекларовано утиліти для візуалізації гістограми в консолі та експорту
// готового аналітичного звіту у файл.


#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <stdio.h>
#include <stdlib.h>

// Створюю структуру даних
typedef struct {
    double min_hist;         // Нижня межа (лівий край графіка)
    double max_hist;         // Верхня межа (правий край графіка)
    unsigned M;              // Кількість стовпчиків
    unsigned* frequency;     // Кількість чисел у кожному стовпчику
    unsigned long total_count; // Облік усіх чисел, які пройшли через систему
} Histogram;

// Етап 1: Ініціалізація та керування пам'яттю

// Ініціалізування гістограми
int initHistogram(Histogram* h, double min_val, double max_val, unsigned m_val);

// Введення з консолі
int histAddFromConsole(Histogram* h, int strategy);

// Звільнювач пам'яті
void destroyHistogram(Histogram* h);


// Етап 2: Модифікатори

// Змінює праву межу діапазону
void histSetMax(Histogram* h, double m);

// Змінює ліву межу діапазону
void histSetMin(Histogram* h, double m);

// Змінює кількість стовпців
void histSetM(Histogram* h, unsigned m);

// Етап 3: Додавання даних

// Додавання по одному числу до відповідного стовпчика
void histAddNumber(Histogram* h, double x, int strategy);

// Обробляє цілий масив чисел
void histAddbatch(Histogram* h, double data[], size_t dataSize, int strategy);

// З текстового файлу
int histAddFromTextFile(Histogram* h, const char* filename, int strategy);

// З бінарного файлу
int histAddFromBinaryFile(Histogram* h, const char* filename, int strategy);

// Зберігання розрахунків у текстовий файл-звіт
void histExportResultsToFile(Histogram* h, const char* filename);

// Етап 4: Математичні функції

// Отримує загальну кількість оброблених значень N
unsigned long histNum(Histogram* h);

// Скільки значень потрапило в конкретний стовпчик i
unsigned histNumHist(Histogram* h, unsigned i);

// Вираховує вибіркове середнє арифметичне (E).
double histMean(Histogram* h);

// Рахує медіану
double histMedian(Histogram* h);

// Рахує дисперсію
double histVariance(Histogram* h);

// Рахує середньоквадратичне відхилення
double histDev(Histogram* h);

// Рахує коефіцієнт асиметрії
double histSkewness(Histogram* h);

// Рахує ексцес
double histKurtosis(Histogram* h);

// Рахує моду
double histMode(Histogram* h);

// Рахує відносну мінливість у відсотках
double variation_coeff(Histogram* h);

// Етап 5: Критерії Пірсона (Перевірка гіпотез)
// Порівнюється отримана гістограма з ідеальними математичними моделями

double histTestPearsonNormal(Histogram* h);      // Перевірка на нормальний розподіл
double histTestPearsonUniform(Histogram* h);     // Перевірка на рівномірний розподіл
double histTestPearsonExponential(Histogram* h); // Перевірка на експоненціальний розподіл
double histTestPearsonPoisson(Histogram* h);     // Перевірка на розподіл Пуассона
double histTestPearsonBinomial(Histogram* h, int n); // Перевірка на біноміальний закон

// Утиліти та візуалізація

// Вираховує фізичну ширину одного стовпчика
double histGetWidth(Histogram* h);

// Знаходить центр i-го стовпчика
double histGetMidpoint(Histogram* h, unsigned i);

// Малює графік гістограми в консолі
void histPrintHistogram(Histogram* h);

// Пише таблицю з усіма статистичними показниками
void histPrintStatistics(Histogram* h);

// Виводить результати тестів Хі-квадрат
void histPrintChiSquare(Histogram* h);

// Чи норм гістограма
int histIsValid(Histogram* h);

#endif