// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Файл реалізації завдання мовою С, що містить повний набір інструментів
// для технічної роботи з гістограмою. Він включає функції для
// виділення пам'яті та ініціалізації структури, методи зчитування
// даних з консолі, текстових та бінарних файлів; алгоритми
// арифметичного розрахунку основних статистичних показників і
// перевірки критеріїв Пірсона (для нормального, експоненційного, біноміального, рівномірного розподілів та розподілу Пуасона.
// Також описана логіка візуалізації графіка в консолі та автоматичного
// формування текстового звіту з результатами аналізу.

#include "Histogram_c.h"
#include <math.h>
#include <string.h>

#define EPSILON_ABS 1e-10
#define EPSILON_REL 1e-6

// Наближення логарифма факторіала через суму логарифмів
// Рятує від переповнення double, коли рахується велике n! для Пуассона/Біноміального.
double lgamma_approx(double x) {
    if (x <= 0) return 0.0;
    double sum = 0.0;
    for (int i = 1; i < (int)x; i++) {
        sum += log((double)i); // вик.власт.логарифмів: ln(a*b) = ln(a) + ln(b)
    }
    return sum;
}

// Перевірка чи гістограма нормальна
int histIsValid(Histogram* h) {
    return h != NULL && h->frequency != NULL && h->M > 0 && h->min_hist < h->max_hist;
}

// Виділяється пам'ять під M стовпців та встановлюються межі
int initHistogram(Histogram* h, double min_val, double max_val, unsigned m_val) {
    if (h == NULL) return 0;

    // Якщо користувач переплутав межі місцями - міняємо їх місцями
    if (min_val > max_val) {
        printf("Warning: Min value was greater than Max value. Swapping them automatically.\n");
        double temp = min_val;
        min_val = max_val;
        max_val = temp;
    }

    // Якщо межі абсолютно однакові - видаємо помилку
    else if (fabs(min_val - max_val) < EPSILON_ABS) {
        // Міняємо fprintf на звичайний printf, щоб текст не вискакував із запізненням
        printf("Error: Min and Max values cannot be equal. Please enter different boundaries.\n");
    }

    // Записуємо валідні межі в структуру
    h->min_hist = min_val;
    h->max_hist = max_val;
    h->M = (m_val > 0) ? m_val : 1;

    // Виділяємо пам'ять динамічно через malloc
    h->frequency = (unsigned*)malloc(h->M * sizeof(unsigned));

    // Перевірка, чи виділилася пам'ять
    if (h->frequency == NULL) {
        fprintf(stderr, "Error: Memory allocation failed!\n");
        exit(1);
    }
    // Занулення
    memset(h->frequency, 0, h->M * sizeof(unsigned));
    h->total_count = 0;
}

// Деструктор. Звільняється пам'ять масиву при видаленні об'єкта
void destroyHistogram(Histogram* h) {
    if (h && h->frequency) {
        free(h->frequency); // Повертається пам'ять системі
        h->frequency = NULL;
    }
}

// Обнуляються частоти, якщо потрібно завантажити нові дані в ту саму структуру
void clearHistogram(Histogram* h) {
    if (!histIsValid(h)) return;
    for (unsigned i = 0; i < h->M; ++i) {
        h->frequency[i] = 0;
    }
    h->total_count = 0;
}

// Сеттери з перевіркою на валідність вводу

// Встановлення правої межі діапазону
void histSetMax(Histogram* h, double m) {
    if (!histIsValid(h)) return;

    if (isnan(m) || isinf(m)) {
        fprintf(stderr, "Error: Invalid max value\n");
        return;
    }
    if (m <= h->min_hist) {
        fprintf(stderr, "Error: max must be greater than min\n");
        return;
    }
    h->max_hist = m;
}

// Встановлення лівої межі діапазону
void histSetMin(Histogram* h, double m) {
    if (!histIsValid(h)) return;

    if (isnan(m) || isinf(m)) {
        fprintf(stderr, "Error: Invalid min value\n");
        return;
    }
    if (m >= h->max_hist) {
        fprintf(stderr, "Error: min must be less than max\n");
        return;
    }
    h->min_hist = m;
}

// Зміна кількості стовпців
void histSetM(Histogram* h, unsigned m) {
    if (!histIsValid(h)) return;

    if (m == 0) {
        fprintf(stderr, "Error: M must be greater than 0\n");
        return;
    }
    if (m == h->M) return;

    if (h->total_count > 0) {
        fprintf(stderr, "Warning: Cannot change M after adding data. Use clearHistogram() first.\n");
        return;
    }

    free(h->frequency); // Видаляється старий масив
    h->M = m;
    h->frequency = (unsigned*)malloc(h->M * sizeof(unsigned)); // Створюється новий
    memset(h->frequency, 0, h->M * sizeof(unsigned));
}

// Рахується ширина одного стовпчика
double histGetWidth(Histogram* h) {
    if (!histIsValid(h)) return 0.0;
    return (h->max_hist - h->min_hist) / h->M;
}

// Знаходження центру і-го інтервалу
double histGetMidpoint(Histogram* h, unsigned i) {
    if (!histIsValid(h) || i >= h->M) return 0.0;
    return h->min_hist + (i + 0.5) * histGetWidth(h);
}

// Додавання одного числа до статистики
void histAddNumber(Histogram* h, double x, int strategy) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Гістограма не валідна\n");
        return;
    }
    if (isnan(x) || isinf(x)) {
        fprintf(stderr, "Warning: Пропускаємо некоректне значення (NaN/Inf)\n");
        return;
    }

    if (x < h->min_hist) {  // Якщо число вилетіло за ліву межу
        if (strategy == 2) { //Якщо стратегія 2 - число йде в перший стовпчик
            h->frequency[0]++;
            h->total_count++;
        }
        return; // Якщо стратегія 1 - число проігнорується
    }

    if (x >= h->max_hist) {  // Якщо число вилетіло за праву межу
        if (strategy == 2) { //Якщо стратегія 2 - число йде в останній стовпчик
            h->frequency[h->M - 1]++;
            h->total_count++;
        }
        return; // Якщо стратегія 1 - число проігнорується
    }

    // Якщо число в межах діапазону — рахую, в який саме стовпчик воно має попасти
    unsigned index = (unsigned)((x - h->min_hist) / histGetWidth(h));

    if (index >= h->M) index = h->M - 1; //індекс не може виходити за межі масиву
    h->frequency[index]++;  // Збільшується лічильник у знайденому стовпчику
    h->total_count++;
}

// Додаю цілий масив чисел
void histAddbatch(Histogram* h, double data[], size_t dataSize, int strategy) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Гістограма не валідна\n");
        return;
    }

    if (data == NULL || dataSize == 0) {
        fprintf(stderr, "Error: Передано порожній масив даних\n");
        return;
    }
    for (size_t i = 0; i < dataSize; ++i) {
        histAddNumber(h, data[i], strategy); // Викликається функція додавання для кожного окремого числа
    }
}

// Читання з файлів

// З текстового файлу
int histAddFromTextFile(Histogram* h, const char* filename, int strategy) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    double val;
    int count = 0;

    // Цикл працює, поки вдається зчитати хоча б одне число
    while (fscanf(f, "%lf", &val) == 1) { // fscanf шукає у файлі числа (double)
        histAddNumber(h, val, strategy); // Кожне знайдене число відправляю в гістограму
        count++;
    }
    fclose(f);

    if (count > 0) {
        printf("Successfully loaded %d values from text file.\n", count);
        return 1;
    }
    return 0;
}

// З бінарного файлу
int histAddFromBinaryFile(Histogram* h, const char* filename, int strategy) {
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;

    double val;
    int count = 0;


    // Цикл працює, поки вдається зчитати хоча б одне число
    // fread копіює сирі байти безпосередньо в пам'ять змінної val
    while (fread(&val, sizeof(double), 1, f) == 1) {
        histAddNumber(h, val, strategy);
        count++;
    }
    fclose(f);

    if (count > 0) {
        printf("Successfully loaded %d values from binary file.\n", count);
        return 1;
    }
    return 0;
}

// Зчитування з консолі
int histAddFromConsole(Histogram* h, int strategy) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Histogram is not valid\n");
        return 0;
    }

    // Показую користувачу межі, щоб він знав, які числа вводити
    printf("\n--- Data Input Mode (Range: [%.2f, %.2f), M: %u) ---\n",
           h->min_hist, h->max_hist, h->M);
    printf("Enter values one per line. Type 'q' to finish:\n");

    double val;
    int count = 0;

    while (1) {
        printf(" Value %d: ", count + 1);
        if (scanf("%lf", &val) != 1) {
            while (getchar() != '\n'); // Очищення буфера від символа
            break;
        }
        histAddNumber(h, val, strategy); // Додаю зчитане число в гістограму
        count++;
    }

    if (count > 0) { // Якщо було введено хоча б щось — виводиться короткий звіт
        printf("\nSuccessfully added %d values.\n", count);
    }
    return count > 0; // Повертаю 1, якщо дані були успішно додані
}

// Повертає загальну кількість N доданих значень
unsigned long histNum(Histogram* h) {
    if (!histIsValid(h)) return 0;
    return h->total_count;
}

// Повертає кількість елементів у і-му стовпці
unsigned histNumHist(Histogram* h, unsigned i) {
    if (!histIsValid(h) || i >= h->M) return 0;
    return h->frequency[i];
}

// Розрахунок вибіркового середнього
double histMean(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    double sum = 0;
    for (unsigned i = 0; i < h->M; ++i) {
        // Додаю добуток к-сті елементів стовпчика на його центральну точку
        sum += h->frequency[i] * histGetMidpoint(h, i);
    }
    return sum/h->total_count; // результат - сума всіх значень, поділена на їх кількість
}

// Обчислення вибіркової дисперсії
double histVariance(Histogram* h) { // для розрахунку дисперсії потрібно хоча б два значення у вибірці
    if (!histIsValid(h) || h->total_count <= 1) return 0.0;

    double m = histMean(h); // середнє арифметичне для розрахунку відхилень
    double sum = 0;

    for (unsigned i = 0; i < h->M; ++i) {
        double diff = histGetMidpoint(h, i) - m; // знаходжу відхилення середини поточного інтервалу від загального середнього
        sum += h->frequency[i] * diff * diff; // сумую квадрати відхилень, зважені на кількість чисел у цьому інтервалі
    }

    // Ділю на (N - 1), щоб отримати несміщену оцінку дисперсії для вибірки
    return sum / (h->total_count - 1);
}

// Обчислення стандартного відхилення
double histDev(Histogram* h) {
    double var = histVariance(h);
    return var >= 0 ? sqrt(var) : 0.0; // корінь квадратний з дисперсії або 0
}

// Розрахунок медіани
double histMedian(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    double half = h->total_count / 2.0;
    double cumulative = 0;

    for (unsigned i = 0; i < h->M; ++i) {
        cumulative += h->frequency[i];

        // Як тільки cumulative став більшим за half, знайшовся потрібний інтервал
        if (cumulative >= half) {
            // Вираховую нижню межу
            double L = h->min_hist + i * histGetWidth(h);

            // Дізнаюсь, скільки значень було зібрано до початку цього стовпчика
            double prev_cum = cumulative - h->frequency[i];

            // Страховка: якщо стовпчик порожній, медіана десь посередині
            if (h->frequency[i] == 0) {
                return L + histGetWidth(h) / 2.0;
            }
            // Визначаю, на яку частку інтервалу треба зайти вглиб, щоб досягти середини вибірки
            return L + histGetWidth(h) * ((half - prev_cum) / h->frequency[i]);
        }
    }
    // Якщо медіана якимось дивом випала за межі
    return h->max_hist;
}

// Коефіцієнт асиметрії (Skewness)
// Показує, наскільки "дзвін" розподілу перекошений в один бік відносно центру
double histSkewness(Histogram* h) {
    if (!histIsValid(h) || h->total_count <= 2 || histDev(h) == 0) return 0.0;
    double m = histMean(h);
    double s = histDev(h);
    double sum = 0;
    for (unsigned i = 0; i < h->M; ++i) {
        // Проводжу стандартизацію відхилення (відстань від середнього у кількостях сигм)
        double tmp = (histGetMidpoint(h, i) - m) / s;
        // Накопичую третій центральний момент: множу куб відхилення на частоту інтервалу
        sum += h->frequency[i] * tmp * tmp * tmp;
    }
    return sum / h->total_count;
}

// Коефіцієнт ексцесу (Kurtosis)
// Показує, наскільки "гострою" є вершина розподілу та наскільки "важкими" є його хвости
double histKurtosis(Histogram* h) { // для розрахунку четвертого моменту потрібно хоча б 4 значення
    if (!histIsValid(h) || h->total_count <= 3 || histDev(h) == 0) return 0.0;
    double m = histMean(h);
    double s = histDev(h);
    double sum = 0;
    for (unsigned i = 0; i < h->M; ++i) {
        // Проводжу стандартизацію відхилення (відстань від середнього у кількостях сигм)
        double tmp = (histGetMidpoint(h, i) - m) / s;
        // Підношу відхилення до четвертого степеня, щоб виділити вплив "важких хвостів" та гостроту вершини
        sum += h->frequency[i] * tmp * tmp * tmp * tmp; // Четвертий центральний момент
    }
    return (sum / h->total_count) - 3.0;
}

// Коефіцієнт варіації (V) у відсотках
// Показує відносну міру розсіювання даних. Це дозволяє порівняти мінливість абсолютно різних явищ
double variation_coeff(Histogram* h) {
    double m = histMean(h);
    double d = histDev(h);

    if (m == 0) return 0.0; // захист від ділення на 0
    // Рахую відношення розкиду до середнього значення і множу на 100.0 для %
    return (d / m) * 100.0;  // якщо V < 33%, вибірка вважається однорідною
}

// Знаходження моди - шукаю інтервал, у який потрапило найбільше чисел
double histMode(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    unsigned max_f = h->frequency[0];
    unsigned mode_idx = 0;

    for (unsigned i = 1; i < h->M; ++i) {
        if (h->frequency[i] > max_f) {
            max_f = h->frequency[i]; // оновлюю максимальну частоту
            mode_idx = i; // фіксую індекс "лідера"
        }
    }
    // Оскільки відомий лише інтервал, то за моду приймається його середина
    return histGetMidpoint(h, mode_idx);
}

// Тест Пірсона (Хі-квадрат) для нормального розподілу
// Перевіряю, чи збігається гістограма з ідеальним "дзвоном" Гаусса
double histTestPearsonNormal(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    double m = histMean(h);
    double s = histDev(h);

    if (s == 0) return 0.0; // якщо розкиду немає - виходжу, щоб не ділити на нуль

    double chi_square = 0;
    for (unsigned i = 0; i < h->M; ++i) {
        if (h->frequency[i] == 0) continue; // Пропускаю порожні стовпчики, вони не дають похибки

        // Нормування меж інтервалу
        // Рахую, на скільки "середніх кроків" (s) ліва та права межі стовпчика віддалені від центру (m)
        double x1 = (h->min_hist + i * histGetWidth(h) - m) / s;
        double x2 = (h->min_hist + (i + 1) * histGetWidth(h) - m) / s;

        // За допомогою спеціальної функції (erf) дізнаюсь, скільки відсотків
        // "ідеальних" даних мало б потрапити в цей проміжок
        double cdf1 = 0.5 * (1.0 + erf(x1 / sqrt(2.0)));
        double cdf2 = 0.5 * (1.0 + erf(x2 / sqrt(2.0)));

        // Множу цей відсоток на загальну кількість чисел (N),
        // щоб отримати очікувану кількість для цього стовпчика
        double expected = (cdf2 - cdf1) * h->total_count;

        if (expected > 0) {
            // Дивлюсь, наскільки реальний стовпчик відрізняється від очікуваного
            // Чим більша ця різниця, тим менше дані схожі на нормальний розподіл
            double diff = h->frequency[i] - expected;
            chi_square += diff * diff / expected;
        }
    }
    return chi_square;
}

// Тест Пірсона для рівномірного розподілу
double histTestPearsonUniform(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;
    // В ідеальному рівномірному розподілі в кожен стовпчик має потрапити
    // однакова кількість чисел: загальну суму (N) ділю на кількість стовпчиків (M)
    double expected_freq = (double)h->total_count / h->M;
    double chi_square = 0;
    for (unsigned i = 0; i < h->M; ++i) {
        // Рахую різницю між реальною частотою стовпчика та теоретично очікуваною
        double diff = h->frequency[i] - expected_freq;
        // Накопичую суму квадратів відхилень, нормованих на очікуване значення (Хі-квадрат)
        chi_square += diff * diff / expected_freq;
    }
    return chi_square; // Чим ближче результат до нуля, тим рівномірніше заповнена гістограма
}

// Тест Пірсона для експоненціального розподілу
double histTestPearsonExponential(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    double m = histMean(h);
    if (m <= 0) return 0.0; // Експоненціальний розподіл працює тільки для додатних значень

    // Параметр лямбда(інтенсивність): чим більше середнє, тим повільніше згасає графік
    double lambda = 1.0 / m;
    double chi_square = 0;

    for (unsigned i = 0; i < h->M; ++i) {
        if (h->frequency[i] == 0) continue; // пропускаю порожні інтервали, щоб не додавати нульову помилку

        double x1 = h->min_hist + i * histGetWidth(h); // Визначаю фізичні межі поточного стовпчика
        double x2 = h->min_hist + (i + 1) * histGetWidth(h);

        // Рахую площу для інтервалу через функцію розподілу (CDF)
        // P = F(x2)-F(x1),де F(x) = 1-e^(-λx)
        double cdf1 = (x1 > 0) ? (1.0 - exp(-lambda * x1)) : 0.0;
        double cdf2 = (x2 > 0) ? (1.0 - exp(-lambda * x2)) : 0.0;
        double p = cdf2 - cdf1;

        if (p <= 0) continue;
        double expected = p * h->total_count; // скільки чисел теоретично мало б бути в цьому стовпчику

        if (expected > 0) {
            double diff = h->frequency[i] - expected; // Міра розбіжності між реальністю та математичною моделлю
            chi_square += diff * diff / expected;
        }
    }
    return chi_square;
}

// Тест Пірсона для розподілу Пуассона
double histTestPearsonPoisson(Histogram* h) {
    if (!histIsValid(h) || h->total_count == 0) return 0.0;

    double L = histMean(h); // Для Пуассона параметр λ дорівнює вибірковому середньому
    if (L <= 0) return 0.0;
    double chi_square = 0;

    for (unsigned i = 0; i < h->M; ++i) {
        if (h->frequency[i] == 0) continue;
        // Оскільки розподіл Пуассона дискретний, округлюю центр інтервалу до цілого числа
        int k = (int)round(histGetMidpoint(h, i));
        if (k < 0) continue;

        // Рахую ймовірність через логарифми, щоб код не "впав" від великих k!
        // Формула: ln(P) = k*ln(λ)-λ-ln(k!)
        double log_p = k * log(L) - L - lgamma_approx(k + 1);
        double p = exp(log_p);
        double expected = p * h->total_count; // скільки значень мало б теоретично бути в цьому стовпчику

        if (expected > 0) {
            // Сумую похибку Хі-квадрат: (факт - теорія)^2 / теорія
            double diff = h->frequency[i] - expected;
            chi_square += diff * diff / expected;
        }
    }
    return chi_square;
}

// Тест Пірсона для біноміального розподілу
double histTestPearsonBinomial(Histogram* h, int n) {
    if (!histIsValid(h) || h->total_count == 0 || h->M <= 1) return 0.0;

    // Кількість випробувань (n) прирівнюю до кількості інтервалів - 1
    int n_bins = (int)h->M - 1;
    // Розраховую ймовірність успіху (p) через положення середнього значення в діапазоні
    double p_prob = (histMean(h) - h->min_hist) / (h->max_hist - h->min_hist);

    // Ймовірність має бути в межах (0, 1), щоб логарифми не видали помилку
    if (p_prob <= 0.0) p_prob = 0.01;
    if (p_prob >= 1.0) p_prob = 0.99;

    double chi_square = 0.0;
    for (unsigned i = 0; i < h->M; ++i) {
        int k = (int)i;
        // Використовую логарифми для розрахунку комбінацій (рятує від величезних чисел)
        double log_Cnk = lgamma_approx(n_bins + 1) - lgamma_approx(k + 1) - lgamma_approx(n_bins - k + 1);
        // Рахую логарифм ймовірності за формулою Бернуллі
        double log_p_k = log_Cnk + k * log(p_prob) + (n_bins - k) * log(1.0 - p_prob);
        // Повертаюсь до реальної ймовірності та множу на N, щоб знайти очікувану частоту
        double p_k = exp(log_p_k);
        double expected = p_k * h->total_count;

        if (expected > 1e-7) { // Рахую Хі-квадрат тільки якщо очікуване значення не мізерне
            // Вираховую різницю між фактичною кількістю значень та теоретично передбаченою
            double diff = (double)h->frequency[i] - expected;
            // Додаю до загального показника Хі-квадрат квадрат відхилення, нормований на очікувану частоту
            chi_square += (diff * diff) / expected;
            }
    }
    return chi_square;
}

// Виведення текстової гістограми безпосередньо в консоль
void histPrintHistogram(Histogram* h) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Histogram is not valid\n");
        return;
    }

    // Виводжу заголовок із загальною кількістю елементів у вибірці
    printf("\nHistogram (N = %lu):\n", h->total_count);
    printf("%-50s\n", "");
    for (int i = 0; i < 50; i++) printf("-");
    printf("\n");

    for (unsigned i = 0; i < h->M; ++i) {
        // Виводжу ліву та праву межі поточного інтервалу разом із частотою потраплянь
        printf("[%6.2f - %6.2f) : %6u | ",
               h->min_hist + i * histGetWidth(h),
               h->min_hist + (i + 1) * histGetWidth(h),
               h->frequency[i]);

        // Рахую довжину горизонтального стовпчика (максимум 30 символів)
        // Масштабую частоту відносно загальної кількості даних для наочності
        int bar_width = (h->frequency[i] * 30) / (h->total_count > 0 ? h->total_count : 1);

        // Малюю графічний бар, використовуючи символ #
        for (int j = 0; j < bar_width; ++j) {
            printf("#");
        }
        printf("\n");
    }
    // Виводжу фінальну роздільну лінію для завершення графіка
    for (int i = 0; i < 50; i++) printf("-");
    printf("\n");
}

// Виведення основних статистичних показників
void histPrintStatistics(Histogram* h) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Histogram is not valid\n");
        return;
    }

    printf("\n=== Statistics ===\n");
    for (int i = 0; i < 40; i++) printf("-");
    printf("\n");

    // Виводжу обсяг вибірки та показники центру (середнє, медіана, мода)
    printf("Count:       %15lu\n", histNum(h));
    printf("Mean (E):    %15.4f\n", histMean(h));
    printf("Median (Me): %15.4f\n", histMedian(h));
    printf("Mode (Mo):   %15.4f\n", histMode(h));

    // Виводжу показники розсіювання (наскільки дані "розкидані")
    printf("Variance:    %15.4f\n", histVariance(h));
    printf("StdDev:      %15.4f\n", histDev(h));

    // Виводжу форму розподілу: нахил (Skewness) та гостроту піка (Kurtosis)
    printf("Skewness:    %15.4f\n", histSkewness(h));
    printf("Kurtosis:    %15.4f\n", histKurtosis(h));

    // Виводжу відносну мінливість у відсотках
    printf("Coeff Var:     %15.4f\n", variation_coeff(h));
    for (int i = 0; i < 40; i++) printf("-");
    printf("\n");
}

// Виведення результатів перевірки статистичних гіпотез
void histPrintChiSquare(Histogram* h) {
    if (!histIsValid(h)) {
        fprintf(stderr, "Error: Histogram is not valid\n");
        return;
    }

    printf("\n=== Chi-Square Tests ===\n");
    for (int i = 0; i < 40; i++) printf("-");
    printf("\n");

    // Виводжу результати порівняння гістограми з ідеальними моделями
    // Чим менше число Хі-квадрат, тим краще дані відповідають цій моделі
    printf("Normal:      %15.6f\n", histTestPearsonNormal(h));
    printf("Uniform:     %15.6f\n", histTestPearsonUniform(h));
    printf("Exponential: %15.6f\n", histTestPearsonExponential(h));
    printf("Poisson:     %15.6f\n", histTestPearsonPoisson(h));
    printf("Binomial (n=M-1): %15.6f\n", histTestPearsonBinomial(h, 0));
    for (int i = 0; i < 40; i++) printf("-");
    printf("\n");
}

// Функція для експорту всіх розрахунків у текстовий файл
void histExportResultsToFile(Histogram* h, const char* filename) {
    if (!histIsValid(h)) return;

    FILE* f = fopen(filename, "w");
    if (!f) return;
    char sep[] = "============================================================";

    // Формую заголовок звіту та записую конфігурацію гістограми (межі, кількість стовпців)
    fprintf(f, "%s\nHISTOGRAM ANALYSIS REPORT (C Version)\n%s\n\n", sep, sep);
    fprintf(f, "CONFIGURATION:\nRange: [%.2f - %.2f)\nBins (M): %u\nTotal (N): %lu\n\n",
            h->min_hist, h->max_hist, h->M, h->total_count);

    // Записую візуалізацію: межі інтервалів, частоти та графічні бари з #
    fprintf(f, "%s\nHISTOGRAM VISUALIZATION\n%s\n", sep, sep);
    for (unsigned i = 0; i < h->M; ++i) {
        fprintf(f, "[%8.2f - %8.2f) : %6u | ",
                h->min_hist + i * histGetWidth(h), h->min_hist + (i + 1) * histGetWidth(h), h->frequency[i]);
        // Розраховую масштаб графіка для текстового файлу
        int bar_width = (h->total_count > 0) ? (int)((h->frequency[i] * 30) / h->total_count) : 0;
        for (int j = 0; j < bar_width; ++j) fprintf(f, "#");
        fprintf(f, "\n");
    }

    // Розраховую та записую основні статистичні показники (середнє, медіану, дисперсію тощо)
    fprintf(f, "\n%s\nSTATISTICAL CHARACTERISTICS\n%s\n", sep, sep);
    fprintf(f, "%-25s %20.4f\n", "Mean (E):", histMean(h));
    fprintf(f, "%-25s %20.4f\n", "Median (Me):", histMedian(h));
    fprintf(f, "%-25s %20.4f\n", "Mode (Mo):", histMode(h));
    fprintf(f, "%-25s %20.4f\n", "Variance:", histVariance(h));
    fprintf(f, "%-25s %20.4f\n", "Std Dev:", histDev(h));
    fprintf(f, "%-25s %20.4f\n", "Skewness:", histSkewness(h));
    fprintf(f, "%-25s %20.4f\n", "Kurtosis:", histKurtosis(h));
    fprintf(f, "%-25s %20.4f\n", "Coeff Var (%):", variation_coeff(h));

    // Виконую перевірку статистичних гіпотез через тест Хі-квадрат для різних розподілів
    fprintf(f, "\n%s\nCHI-SQUARE HYPOTHESIS TESTS\n%s\n", sep, sep);
    fprintf(f, "%-25s %20.6f\n", "Normal:", histTestPearsonNormal(h));
    fprintf(f, "%-25s %20.6f\n", "Uniform:", histTestPearsonUniform(h));
    fprintf(f, "%-25s %20.6f\n", "Exponential:", histTestPearsonExponential(h));
    fprintf(f, "%-25s %20.6f\n", "Poisson:", histTestPearsonPoisson(h));
    fprintf(f, "%-25s %20.6f\n", "Binomial:", histTestPearsonBinomial(h, 0));

    fprintf(f, "%s\n", sep);
    // Закриваю файл, зберігаючи всі внесені дані на диск
    fclose(f);
}