// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Головний файл програми мовою С, який містить функцію для автоматичної
// генерації вибірок за різними законами розподілу (нормальним,
// рівномірним, експоненціальним тощо) та реалізацію інтерактивного меню
// для вибору режимів роботи. Файл координує процеси налаштування параметрів гістограми,
// зчитування даних, запуск арифметичних розрахунків статистики та збереження
// фінального аналітичного звіту.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "Histogram_c.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Допоміжна функція для очищення буфера вводу.
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Функція для штучної генерації даних за різними законами розподілу
void generateDataC(const char* filename, int isBinary, int distType, int count, double min, double max) {
    // isBinary == 1 - wb, якщо ні w
    FILE* f = isBinary ? fopen(filename, "wb") : fopen(filename, "w");
    if (!f) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }

    // Ініціалізую генератор випадкових чисел поточним часом (time(NULL))
    srand((unsigned)time(NULL));

    double range = max - min; //Ширина діапазону
    double mid = min + range / 2.0; //Центральна точка

    for (int i = 0; i < count; i++) {
        double val = 0;

        // Функція rand() видає ціле число від 0 до константи RAND_MAX
        // Ділю rand() на RAND_MAX, щоб отримати випадкове дробове число
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;

        // Якщо u1 (логарифм) стане нулем -> заміна на мале число
        if (u1 <= 0) u1 = 1e-9;

        // Залежно від вибору користувача (distType), застосовую різні математичні формули
        switch (distType) {
            case 1:
                // Нормальний розподіл. Використовую перетворення Бокса-Мюллера,
                // яке з двох рівномірних чисел (u1, u2) робить одне "нормальне"
                // Множу на (range / 6.0), щоб за "правилом трьох сигм" 99% чисел влізли в межі
                val = mid + (range / 6.0) * sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
                break;
            case 2:
                // Рівномірний розподіл. Беру число від 0 до 1 (u1),
                // множу на ширину діапазону і додаю мінімум
                val = min + u1 * range;
                break;
            case 3:
                // Експоненціальний розподіл. Використовую метод обернених функцій.
                // Формула містить логарифм. Чим ближче u1 до нуля, тим більшим вийде значення.
                val = min - log(u1) * (range / 3.0);
                break;
            case 4:
                // Розподіл Пуассона (кількість рідкісних подій).
                // Встановлюю параметр лямбда (L_val), але обмежую його числом 30,
                // бо інакше експонента exp(-L_val) стане такою малою, що округлиться
                // до нуля, і цикл стане нескінченним.
                {
                    double L_val = (range / 4.0) > 30 ? 30 : (range / 4.0);
                    double L_exp = exp(-L_val), p = 1.0;
                    int k = 0;
                    // Множу випадкові числа, поки їх добуток не стане меншим за L_exp.
                    // Кількість кроків (k) - число Пуассона.
                    do { k++; p *= (double)rand() / RAND_MAX; } while (p > L_exp);
                    val = min + (double)(k - 1);
                }
                break;
            case 5:
                // Біноміальний розподіл. Генерую нормальний розподіл і округлюю
                // його до найближчого цілого числа за допомогою функції floor().
                val = floor(mid + (range / 10.0) * sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2) + 0.5);
                break;
            default:
                val = min + u1 * range;
                break;
        }

        // Якщо isBinary істинне - fwrite, якщо ні - запис як звичайний текст
        if (isBinary) fwrite(&val, sizeof(double), 1, f);
        else fprintf(f, "%.4f\n", val);
    }
    fclose(f);
}

int main() {
    int choice;

    // Вибір режиму роботи
    printf("Select work mode:\n1. Console input\n2. Text file \n3. Binary file \nChoice: ");

    // scanf повертає кількість змінних, які їй вдалося успішно прочитати
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Please enter 1, 2 or 3: ");
        clearBuffer();
    }
    clearBuffer();

    // Оголошуємо змінні для гістограми
    Histogram h;
    int is_initialized = 0;
    double min_h, max_h;
    int m_bins;
    // Цикл захисту: триває, поки initHistogram не поверне 1
    while (!is_initialized) {
        printf("\nConfigure Histogram:\n");
        printf("Enter Min value: ");
        while (scanf("%lf", &min_h) != 1) { printf("Invalid min. Enter number: "); clearBuffer(); }
        printf("Enter Max value: ");
        while (scanf("%lf", &max_h) != 1) { printf("Invalid max. Enter number: "); clearBuffer(); }
        printf("Enter number of bins (M): ");
        while (scanf("%d", &m_bins) != 1 || m_bins <= 0) { printf("Invalid M. Enter positive number: "); clearBuffer(); }
        clearBuffer();

        // Спроба ініціалізації. Якщо межі однакові - поверне 0, і цикл піде на нове коло
        is_initialized = initHistogram(&h, min_h, max_h, (unsigned)m_bins);
        if (!is_initialized) {
            printf("Let's try configuring the boundaries again.\n");
        }
    }


    char fname[256] = "generated_data_c";

    // Якщо обрано файл - генеруємо дані під наші межі
    if (choice == 2 || choice == 3) {
        int dist, n;
        // Цикл для перевірки коректності вибору розподілу
        while (1) {
            printf("\nSelect distribution:\n1. Normal\n2. Uniform\n3. Exponential\n4. Poisson\n5. Binomial\n6. Random\nChoice: ");

            // перевірка, чи вдалося зчитати ціле число
            if (scanf("%d", &dist) != 1) {
                printf("Error: Please enter a number.\n");
                clearBuffer();
                continue;
            }

            if (dist >= 1 && dist <= 6) {
                break;
            } else {
                printf("Error: Choice %d is invalid. Select between 1 and 6.\n", dist);
                clearBuffer();
            }
        }

        printf("Enter number of elements to generate: ");
        while (scanf("%d", &n) != 1 || n <= 0) {
            printf("Enter positive number: ");
            clearBuffer();
        }
        clearBuffer();

        // Додаю розширення до імені файлу
        // Функція strcat "приклеює" рядок до кінця fname
        if (choice == 2) strcat(fname, ".txt");
        else strcat(fname, ".dat");

        // Використовуємо вже відкориговані межі зі структури h
        generateDataC(fname, (choice == 3), dist, n, h.min_hist, h.max_hist);
        printf("Data generated in %s\n", fname);
    }

    // Завантаження даних у гістограму
    if (choice == 1) {
        // Дізнаюсь в користувача стратегію обробки "викидів"
        int strategy;
        printf("\n--- Out-of-range values handling ---\n1. Discard \n2. Clamp \n");

        while (1) {
            printf("Choice (1 or 2): ");
            if (scanf("%d", &strategy) == 1 && (strategy == 1 || strategy == 2)) {
                clearBuffer();
                break;
            } else {
                printf("Invalid choice. Please enter 1 or 2.\n");
                clearBuffer();
            }
        }
        // Передаю адресу гістограми в функцію ручного вводу
        histAddFromConsole(&h, strategy);
    } else if (choice == 2) {
        // Завантажую дані з текстового файлу
        histAddFromTextFile(&h, fname, 1);
    } else {
        // Завантажую дані з бінарного файлу
        histAddFromBinaryFile(&h, fname, 1);
    }


    //Аналіз та виведення результатів
    histPrintHistogram(&h);
    histPrintStatistics(&h);
    histPrintChiSquare(&h);

    // Записую всі результати у зручний текстовий звіт для користувача
    const char* reportName = "analysis_report_c.txt";
    histExportResultsToFile(&h, reportName);
    printf("\n[OK] Analysis report saved to: %s\n", reportName);

    destroyHistogram(&h);
}