// Назва завдання: Histogram
// Виконували: Сердюк Софія, Бушуєва Дарія
// Спеціальність: Статистика
// Опис файлу:
// Файл реалізації С++ класу Histogram_cpp, що містить основну логіку
// обробки даних у просторі імен cpp_hist

#include "Histogram_cpp.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <cstring>

namespace cpp_hist {
    // Рахує логарифм комбінацій C(n, k)
    // Використовую lgamma, щоб не вилітати за межі double при великих факторіалах
    double log_binomial_coeff(int n, int k) {
        if (k < 0 || k > n) return 0.0;
        if (k == 0 || k == n) return 0.0;
        // Формула через гамма-функції: ln(n!) - ln(k!) - ln((n-k)!)
        return std::lgamma(n + 1.0) - std::lgamma(k + 1.0) - std::lgamma(n - k + 1.0);
    }

    // Виділяється пам'ять під M стовпців та встановлюються межі
    Histogram_cpp::Histogram_cpp(double min_val, double max_val, unsigned m_val) {
        if (min_val > max_val) {
            std::cout << "Warning: Min value was greater than Max value. Swapping them automatically.\n";
            std::swap(min_val, max_val);
        }
        // Якщо межі абсолютно однакові - кидаємо виняток, який зловимо в main
        else if (std::abs(min_val - max_val) < 1e-10) {
            throw std::invalid_argument("Error: Min and Max values cannot be equal. Please enter different boundaries.");
        }

        min_hist = min_val;
        max_hist = max_val;
        M = m_val > 0 ? m_val : 1;
        frequency = new unsigned[M]{0};
        total_count = 0;
    }


    // Деструктор. Звільняється пам'ять масиву при видаленні об'єкта
    Histogram_cpp::~Histogram_cpp() {
        if (frequency) {
            delete[] frequency; // Повертається пам'ять системі
            frequency = nullptr;
        }
    }

    // Перевірка чи гістограма нормальна
    bool Histogram_cpp::isValid() const {
        return frequency != nullptr && M > 0 && min_hist < max_hist;
    }

    // Обнуляються частоти, якщо потрібно завантажити нові дані в ту саму структуру
    void Histogram_cpp::clear() {
        if (frequency) {
            for (unsigned i = 0; i < M; ++i) {
                frequency[i] = 0;
            }
        }
        total_count = 0;
    }

    // Рахується ширина одного стовпчика
    double Histogram_cpp::getWidth() const {
        if (!isValid()) return 0.0;
        return (max_hist - min_hist) / M;
    }

    // Знаходження центру і-го інтервалу
    double Histogram_cpp::getMidpoint(unsigned i) const {
        if (!isValid() || i >= M) return 0.0;
        return min_hist + (i + 0.5) * getWidth();
    }

    // Встановлення правої межі діапазону
    void Histogram_cpp::setMax(double m) {
        if (std::isnan(m) || std::isinf(m)) {
            std::cerr << "Error: Invalid max value\n";
            return;
        }
        if (m <= min_hist) {
            std::cerr << "Error: max must be greater than min\n";
            return;
        }
        max_hist = m;
    }

    // Встановлення лівої межі діапазону
    void Histogram_cpp::setMin(double m) {
        if (std::isnan(m) || std::isinf(m)) {
            std::cerr << "Error: Invalid min value\n";
            return;
        }
        if (m >= max_hist) {
            std::cerr << "Error: min must be less than max\n";
            return;
        }
        min_hist = m;
    }

    // Зміна кількості стовпців
    void Histogram_cpp::setM(unsigned m) {
        if (m == 0) {
            std::cerr << "Error: M must be greater than 0\n";
            return;
        }
        if (m == M) return;

        if (total_count > 0) {
            std::cerr << "Warning: Cannot change M after adding data. Use clear() first\n";
            return;
        }

        delete[] frequency; // Видаляється старий масив
        M = m;
        frequency = new unsigned[M]{0}; // Створюється новий
    }

    // Додавання одного числа до статистики
    void Histogram_cpp::addNumber(double x, int strategy) {
        if (!isValid()) {
            std::cerr << "Error: Гістограма не валідна\n";
            return;
        }
        if (std::isnan(x) || std::isinf(x)) {
            return;
        }
        if (x < min_hist) { // Якщо число вилетіло за ліву межу
            if (strategy == 2) { // Якщо стратегія 2 - число йде в перший стовпчик
                frequency[0]++;
                total_count++;
            }
            return; // Якщо стратегія 1 - число проігнорується
        }
        if (x >= max_hist) { // Якщо число вилетіло за праву межу
            if (strategy == 2) { // Якщо стратегія 2 - число йде в останній стовпчик
                frequency[M - 1]++;
                total_count++;
            }
            return; // Якщо стратегія 1 - число проігнорується
        }
        // Якщо число в межах діапазону — рахую, в який саме стовпчик воно має попасти
        unsigned index = static_cast<unsigned>((x - min_hist) / getWidth());
        if (index >= M) index = M - 1; // індекс не може виходити за межі масиву
        frequency[index]++; // Збільшується лічильник у знайденому стовпчику
        total_count++;
    }

    // Додаю цілий масив чисел
    void Histogram_cpp::addbatch(double data[], size_t dataSize, int strategy) {
        if (!isValid()) {
            std::cerr << "Error: Гістограма не валідна\n";
            return;
        }
        if (data == nullptr || dataSize == 0) {
            std::cerr << "Error: Передано порожній масив даних\n";
            return;
        }
        for (size_t i = 0; i < dataSize; ++i) {
            addNumber(data[i], strategy); // Викликається функція додавання для кожного окремого числа
        }
    }

    // Читання з файлів

    // З текстового файлу
    bool Histogram_cpp::addFromTextFile(const char* filename, int strategy) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open text file '" << filename << "'\n";
            return false;
        }
        double val;
        int count = 0;
        // Цикл працює, поки вдається зчитати хоча б одне число
        while (file >> val) { // шукає у файлі числа (double)
            if (!std::isnan(val) && !std::isinf(val)) {
                addNumber(val, strategy); // Кожне знайдене число відправляю в гістограму
                count++;
            }
        }
        file.close();
        if (count > 0) {
            std::cout << "Successfully loaded " << count << " values from text file\n";
            return true;
        }
        return false;
    }

    // З бінарного файлу
    bool Histogram_cpp::addFromBinaryFile(const char* filename, int strategy) {
        std::ifstream file(filename, std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "Error: Cannot open binary file '" << filename << "'\n";
            return false;
        }
        double val;
        int count = 0;
        // Цикл працює, поки вдається зчитати хоча б одне число
        // read копіює сирі байти безпосередньо в пам'ять змінної val
        while (file.read(reinterpret_cast<char*>(&val), sizeof(double))) {
            if (!std::isnan(val) && !std::isinf(val)) {
                addNumber(val, strategy);
                count++;
            }
        }
        file.close();
        if (count > 0) {
            std::cout << "Successfully loaded " << count << " values from binary file\n";
            return true;
        }
        return false;
    }

    // Зчитування з консолі

    bool Histogram_cpp::addFromConsole(int strategy) {
        if (!isValid()) {
            std::cerr << "Error: Histogram is not valid\n";
            return false;
        }
        // Показую користувачу межі, щоб він знав, які числа вводити
        std::cout << "\n--- Data Input Mode (Range: [" << min_hist << ", " << max_hist << "), M: " << M << ") ---\n";
        std::cout << "Enter values one per line. Type 'q' to finish:\n";

        double val;
        int count = 0;
        std::string input;

        while (true) {
            std::cout << " Value " << (count + 1) << ": ";
            if (!(std::cin >> input)) break;
            if (input == "q") break; // Очищення буфера від символа q

            try {
                val = std::stod(input); // Перетворює введений текст на дробове число
                addNumber(val, strategy); // Додає зчитане число в гістограму
                count++;
            } catch (...) {  // Якщо введені букви або некоректні дані - ловить помилку
                break;
            }
        }
        if (count > 0) { // Якщо було введено хоча б щось - виводиться короткий звіт
            std::cout << "\nSuccessfully added " << count << " values\n";
        }
        return count > 0; // Повертаю 1, якщо дані були успішно додані
    }

    // Статистика

    // Повертає загальну кількість N доданих значень
    unsigned long Histogram_cpp::num() const {
        return total_count;
    }

    // Повертає кількість елементів у і-му стовпці
    unsigned Histogram_cpp::numHist(unsigned i) const {
        if (!isValid() || i >= M) return 0;
        return frequency[i];
    }

    // Розрахунок вибіркового середнього
    double Histogram_cpp::mean() const {
        if (total_count == 0) return 0.0;
        double sum = 0;
        for (unsigned i = 0; i < M; ++i) {
            // Додаю добуток к-сті елементів стовпчика на його центральну точку
            sum += frequency[i] * getMidpoint(i);
        }
        return sum / total_count; // результат - сума всіх значень, поділена на їх кількість
    }

    // Обчислення вибіркової дисперсії
    double Histogram_cpp::variance() const { // для розрахунку дисперсії потрібно хоча б два значення у вибірці
        if (total_count <= 1) return 0.0;
        double m = mean(); // середнє арифметичне для розрахунку відхилень
        double sum = 0;
        for (unsigned i = 0; i < M; ++i) {
            double diff = getMidpoint(i) - m; // знаходжу відхилення середини поточного інтервалу від загального середнього
            sum += frequency[i] * diff * diff; // сумую квадрати відхилень, зважені на кількість чисел у цьому інтервалі
        }
        // Ділю на (N - 1), щоб отримати несміщену оцінку дисперсії для вибірки
        return sum / (total_count - 1);
    }

    // Обчислення стандартного відхилення
    double Histogram_cpp::dev() const {
        double var = variance();
        return var >= 0 ? std::sqrt(var) : 0.0; // корінь квадратний з дисперсії або 0
    }

    // Розрахунок медіани
    double Histogram_cpp::median() const {
        if (total_count == 0) return 0.0;

        double half = total_count / 2.0;
        double cumulative = 0;

        for (unsigned i = 0; i < M; ++i) {
            cumulative += frequency[i];
            // Як тільки cumulative став більшим за half, знайшовся потрібний інтервал
            if (cumulative >= half) {
                // Вираховую нижню межу
                double L = min_hist + i * getWidth();
                // Дізнаюсь, скільки значень було зібрано до початку цього стовпчика
                double prev_cum = cumulative - frequency[i];
                // Страховка: якщо стовпчик порожній, медіана десь посередині
                if (frequency[i] == 0) {
                    return L + getWidth() / 2.0;
                }
                // Визначаю, на яку частку інтервалу треба зайти вглиб, щоб досягти середини вибірки
                return L + getWidth() * ((half - prev_cum) / frequency[i]);
            }
        }
        // Якщо медіана якимось дивом випала за межі
        return max_hist;
    }

    // Коефіцієнт асиметрії (Skewness)
    // Показує, наскільки "дзвін" розподілу перекошений в один бік відносно центру
    double Histogram_cpp::skewness() const {
        if (total_count <= 2 || dev() == 0) return 0.0;
        double m = mean();
        double s = dev();
        double sum = 0;
        for (unsigned i = 0; i < M; ++i) {
            // Проводжу стандартизацію відхилення (відстань від середнього у кількостях сигм)
            double tmp = (getMidpoint(i) - m) / s;
            // Накопичую третій центральний момент: множу куб відхилення на частоту інтервалу
            sum += frequency[i] * std::pow(tmp, 3);
        }
        return sum / total_count;
    }
    // Коефіцієнт ексцесу (Kurtosis)
    // Показує, наскільки "гострою" є вершина розподілу та наскільки "важкими" є його хвости
    double Histogram_cpp::kurtosis() const { // для розрахунку четвертого моменту потрібно хоча б 4 значення
        if (total_count <= 3 || dev() == 0) return 0.0;
        double m = mean();
        double s = dev();
        double sum = 0;
        for (unsigned i = 0; i < M; ++i) {
            // Проводжу стандартизацію відхилення (відстань від середнього у кількостях сигм)
            double tmp = (getMidpoint(i) - m) / s;
            // Підношу відхилення до четвертого степеня, щоб виділити вплив "важких хвостів" та гостроту вершини
            sum += frequency[i] * std::pow(tmp, 4); // Четвертий центральний момент
        }
        return (sum / total_count) - 3.0;
    }
    // Коефіцієнт варіації (V) у відсотках
    // Показує відносну міру розсіювання даних. Це дозволяє порівняти мінливість абсолютно різних явищ
    double Histogram_cpp::coeffVariation() const {
        double m = mean();
        if (m == 0) return 0.0; // захист від ділення на 0
        // Рахую відношення розкиду до середнього значення і множу на 100.0 для %
        return (dev() / m) * 100.0;
    }
    // Знаходження моди - шукаю інтервал, у який потрапило найбільше чисел
    double Histogram_cpp::mode() const {
        if (total_count == 0) return 0.0;

        unsigned max_f = frequency[0];
        unsigned mode_idx = 0;

        for (unsigned i = 1; i < M; ++i) {
            if (frequency[i] > max_f) {
                max_f = frequency[i]; // оновлюю максимальну частоту
                mode_idx = i; // фіксую індекс "лідера"
            }
        }
        // Оскільки відомий лише інтервал, то за моду приймається його середина
        return getMidpoint(mode_idx);
    }

    // Критерії Пірсона

    // Тест Пірсона (Хі-квадрат) для нормального розподілу
    // Перевіряю, чи збігається гістограма з ідеальним "дзвоном" Гаусса
    double Histogram_cpp::testPearsonNormal() const {
        if (total_count == 0 || !isValid()) return 0.0;

        double m = mean();
        double s = dev();
        if (s == 0) return 0.0; // якщо розкиду немає - виходжу, щоб не ділити на нуль

        double chi_square = 0;
        for (unsigned i = 0; i < M; ++i) {
            if (frequency[i] == 0) continue; // Пропускаю порожні стовпчики, вони не дають похибки

            // Нормування меж інтервалу
            // Рахую, на скільки "середніх кроків" (s) ліва та права межі стовпчика віддалені від центру (m)
            double x1 = (min_hist + i * getWidth() - m) / s;
            double x2 = (min_hist + (i + 1) * getWidth() - m) / s;

            // За допомогою спеціальної функції (erf) дізнаюсь, скільки відсотків
            // "ідеальних" даних мало б потрапити в цей проміжок
            double cdf1 = 0.5 * (1.0 + std::erf(x1 / std::sqrt(2.0)));
            double cdf2 = 0.5 * (1.0 + std::erf(x2 / std::sqrt(2.0)));

            // Множу цей відсоток на загальну кількість чисел (N),
            // щоб отримати очікувану кількість для цього стовпчика
            double expected = (cdf2 - cdf1) * total_count;
            if (expected > 0) {
                // Дивлюсь, наскільки реальний стовпчик відрізняється від очікуваного
                // Чим більша ця різниця, тим менше дані схожі на нормальний розподіл
                chi_square += std::pow(frequency[i] - expected, 2) / expected;
            }
        }
        return chi_square;
    }

    // Тест Пірсона для рівномірного розподілу
    double Histogram_cpp::testPearsonUniform() const {
        if (total_count == 0 || !isValid()) return 0.0;
        // В ідеальному рівномірному розподілі в кожен стовпчик має потрапити
        // однакова кількість чисел: загальну суму (N) ділю на кількість стовпчиків (M)
        double expected_freq = static_cast<double>(total_count) / M;
        double chi_square = 0;
        for (unsigned i = 0; i < M; ++i) {
            // Рахую різницю між реальною частотою стовпчика та теоретично очікуваною
            double diff = static_cast<double>(frequency[i]) - expected_freq;
            // Накопичую суму квадратів відхилень, нормованих на очікуване значення (Хі-квадрат)
            chi_square += diff * diff / expected_freq;
        }
        return chi_square; // Чим ближче результат до нуля, тим рівномірніше заповнена гістограма
    }

    // Тест Пірсона для експоненціального розподілу
    double Histogram_cpp::testPearsonExponential() const {
        if (total_count == 0 || !isValid()) return 0.0;

        double m = mean();
        if (m <= 0) return 0.0; // Експоненціальний розподіл працює тільки для додатних значень

        // Параметр лямбда(інтенсивність): чим більше середнє, тим повільніше згасає графік
        double lambda = 1.0 / m;
        double chi_square = 0;

        for (unsigned i = 0; i < M; ++i) {
            if (frequency[i] == 0) continue; // пропускаю порожні інтервали, щоб не додавати нульову помилку

            double x1 = min_hist + i * getWidth(); // Визначаю фізичні межі поточного стовпчика
            double x2 = min_hist + (i + 1) * getWidth();

            // Рахую площу для інтервалу через функцію розподілу (CDF)
            // P = F(x2)-F(x1),де F(x) = 1-e^(-λx)
            double cdf1 = (x1 > 0) ? (1.0 - std::exp(-lambda * x1)) : 0.0;
            double cdf2 = (x2 > 0) ? (1.0 - std::exp(-lambda * x2)) : 0.0;
            double p = cdf2 - cdf1;

            if (p <= 0) continue;
            double expected = p * total_count; // скільки чисел теоретично мало б бути в цьому стовпчику

            if (expected > 0) {
                // Міра розбіжності між реальністю та математичною моделлю
                chi_square += std::pow(frequency[i] - expected, 2) / expected;
            }
        }
        return chi_square;
    }

    // Тест Пірсона для розподілу Пуассона
    double Histogram_cpp::testPearsonPoisson() const {
        if (total_count == 0 || !isValid()) return 0.0;

        double L = mean(); // Для Пуассона параметр λ дорівнює вибірковому середньому
        if (L <= 0) return 0.0;

        double chi_square = 0;
        for (unsigned i = 0; i < M; ++i) {
            if (frequency[i] == 0) continue;
            // Оскільки розподіл Пуассона дискретний, округлюю центр інтервалу до цілого числа
            int k = static_cast<int>(std::round(getMidpoint(i)));
            if (k < 0) continue;

            // Рахую ймовірність через логарифми, щоб код не "впав" від великих k!
            // Формула: ln(P) = k*ln(λ)-λ-ln(k!)
            double log_p = k * std::log(L) - L - std::lgamma(k + 1.0);
            double p = std::exp(log_p);
            double expected = p * total_count; // скільки значень мало б теоретично бути в цьому стовпчику

            if (expected > 0) {
                // Сумую похибку Хі-квадрат: (факт - теорія)^2 / теорія
                chi_square += std::pow(frequency[i] - expected, 2) / expected;
            }
        }
        return chi_square;
    }
    // Тест Пірсона для біноміального розподілу
    double Histogram_cpp::testPearsonBinomial() const {
        if (total_count == 0 || !isValid() || M <= 1) return 0.0;

        // Кількість випробувань (n) прирівнюю до кількості інтервалів - 1
        int n = static_cast<int>(M) - 1;
        // Розраховую ймовірність успіху (p) через положення середнього значення в діапазоні
        double p_prob = (mean() - min_hist) / (max_hist - min_hist);

        // Ймовірність має бути в межах (0, 1), щоб логарифми не видали помилку
        if (p_prob <= 0.0) p_prob = 0.01;
        if (p_prob >= 1.0) p_prob = 0.99;

        double chi_square = 0.0;

        for (unsigned i = 0; i < M; ++i) {
            int k = static_cast<int>(i); // Перетворюю індекс стовпчика у ціле число для розрахунків
            // Використовую логарифми для розрахунку комбінацій (рятує від величезних чисел)
            // ln(P) = ln(C(n,k)) + k*ln(p) + (n-k)*ln(1-p)
            double log_p_k = log_binomial_coeff(n, k) +
                             k * std::log(p_prob) +
                             (n - k) * std::log(1.0 - p_prob);
            // Повертаюсь до реальної ймовірності через експоненту
            double p_k = std::exp(log_p_k);
            // Множу ймовірність на загальну кількість N, щоб знайти очікувану частоту для інтервалу
            double expected = p_k * total_count;

            if (expected > 1e-7) { // Рахую Хі-квадрат тільки якщо очікуване значення не мізерне
                // Вираховую різницю між фактичною кількістю значень та теоретично передбаченою
                double observed = static_cast<double>(frequency[i]);
                // Додаю до загального показника Хі-квадрат квадрат відхилення, нормований на очікувану частоту
                chi_square += std::pow(observed - expected, 2) / expected;
            }
        }
        return chi_square;
    }

    // Виведення

    // Виведення текстової гістограми безпосередньо в консоль
    void Histogram_cpp::printHistogram() const {
        if (!isValid()) {
            std::cerr << "Error: Histogram is not valid\n";
            return;
        }
        // Виводжу заголовок із загальною кількістю елементів у вибірці
        std::cout << "\nHistogram (N = " << total_count << "):\n";
        std::cout << std::string(60, '-') << "\n";

        for (unsigned i = 0; i < M; ++i) {
            // Виводжу ліву та праву межі поточного інтервалу разом із частотою потраплянь
            std::cout << "[" << std::setw(8) << std::fixed << std::setprecision(2)
                      << min_hist + i * getWidth()
                      << " - " << std::setw(8) << min_hist + (i + 1) * getWidth()
                      << ") : " << std::setw(6) << frequency[i] << " | ";
            // Рахую довжину горизонтального стовпчика (максимум 40 символів)
            // Масштабую частоту відносно загальної кількості даних для наочності
            int bar_width = (frequency[i] * 40) / (total_count > 0 ? total_count : 1);
            // Малюю графічний бар, використовуючи символ #
            for (int j = 0; j < bar_width; ++j) std::cout << "#";
            std::cout << "\n";
        }
        // Виводжу фінальну роздільну лінію для завершення графіка
        std::cout << std::string(60, '-') << "\n\n";
    }
    // Виведення основних статистичних показників
    void Histogram_cpp::printStatistics() const {
        if (!isValid() || total_count == 0) return;

        std::cout << "=== Statistics ===\n";
        std::cout << std::string(40, '-') << "\n";

        // Виводжу обсяг вибірки та показники центру (середнє, медіана, мода)
        std::cout << "Count:       " << std::setw(15) << num() << "\n";
        std::cout << "Mean (E):    " << std::setw(15) << std::fixed << std::setprecision(4) << mean() << "\n";
        std::cout << "Median (Me): " << std::setw(15) << median() << "\n";
        std::cout << "Mode (Mo):   " << std::setw(15) << mode() << "\n";

        // Виводжу показники розсіювання (наскільки дані "розкидані")
        std::cout << "Variance:    " << std::setw(15) << variance() << "\n";
        std::cout << "StdDev:      " << std::setw(15) << dev() << "\n";

        // Виводжу форму розподілу: нахил (Skewness) та гостроту піка (Kurtosis)
        std::cout << "Skewness:    " << std::setw(15) << skewness() << "\n";
        std::cout << "Kurtosis:    " << std::setw(15) << kurtosis() << "\n";

        // Виводжу відносну мінливість у відсотках
        std::cout << "Coeff Var:   " << std::setw(15) << coeffVariation() << "\n";
        std::cout << std::string(40, '-') << "\n\n";
    }
    // Виведення результатів перевірки статистичних гіпотез
    void Histogram_cpp::printChiSquare() const {
        if (!isValid() || total_count == 0) return;

        std::cout << "=== Chi-Square Tests ===\n";
        std::cout << std::string(40, '-') << "\n";

        // Виводжу результати порівняння гістограми з ідеальними моделями
        // Чим менше число Хі-квадрат, тим краще дані відповідають цій моделі
        std::cout << "Normal:      " << std::setw(15) << std::fixed << std::setprecision(6) << testPearsonNormal() << "\n";
        std::cout << "Uniform:     " << std::setw(15) << testPearsonUniform() << "\n";
        std::cout << "Exponential: " << std::setw(15) << testPearsonExponential() << "\n";
        std::cout << "Poisson:     " << std::setw(15) << testPearsonPoisson() << "\n";
        std::cout << "Binomial (n=M-1): " << std::setw(15) << testPearsonBinomial() << "\n";
        std::cout << std::string(40, '-') << "\n\n";
    }
    // Функція для експорту всіх розрахунків у текстовий файл
    void Histogram_cpp::exportResultsToFile(const std::string& filename) {
        if (!isValid()) return;
        std::ofstream fout(filename);
        if (!fout.is_open()) return;

        std::string sep(60, '=');
        // Формую заголовок звіту та записую конфігурацію гістограми (межі, кількість стовпців)
        fout << sep << "\nHISTOGRAM ANALYSIS REPORT (C++ Version)\n" << sep << "\n\n";
        fout << "CONFIGURATION:\nRange: [" << min_hist << " - " << max_hist << ")\nBins (M): " << M << "\nTotal (N): " << total_count << "\n\n";

        // Записую візуалізацію: межі інтервалів, частоти та графічні бари з #
        fout << sep << "\nHISTOGRAM VISUALIZATION\n" << sep << "\n";
        for (unsigned i = 0; i < M; ++i) {
            fout << "[" << std::setw(8) << std::fixed << std::setprecision(2) << min_hist + i * getWidth() << " - " << std::setw(8) << min_hist + (i + 1) * getWidth() << ") : "
                 << std::setw(6) << frequency[i] << " | ";
            // Розраховую масштаб графіка для текстового файлу
            int bar_width = (total_count > 0 ? (frequency[i] * 30 / total_count) : 0);
            fout << std::string(bar_width, '#') << "\n";
        }

        // Розраховую та записую основні статистичні показники (середнє, медіану, дисперсію тощо)
        fout << "\n" << sep << "\nSTATISTICAL CHARACTERISTICS\n" << sep << "\n";
        auto row = [&](std::string l, double v, int p = 4) {
            fout << std::left << std::setw(25) << l << std::right << std::fixed << std::setprecision(p) << std::setw(20) << v << "\n";
        };

        row("Mean (E):", mean());
        row("Median (Me):", median());
        row("Mode (Mo):", mode());
        row("Variance:", variance());
        row("Std Dev:", dev());
        row("Skewness:", skewness());
        row("Kurtosis:", kurtosis());
        row("Coeff Var (%):", coeffVariation());

        // Виконую перевірку статистичних гіпотез через тест Хі-квадрат для різних розподілів
        fout << "\n" << sep << "\nCHI-SQUARE HYPOTHESIS TESTS\n" << sep << "\n";
        row("Normal:", testPearsonNormal(), 6);
        row("Uniform:", testPearsonUniform(), 6);
        row("Exponential:", testPearsonExponential(), 6);
        row("Poisson:", testPearsonPoisson(), 6);
        row("Binomial (n=M-1):", testPearsonBinomial(), 6);
        fout << sep << "\n";

        // Закриваю файл, зберігаючи всі внесені дані на диск
        fout.close();
    }
} // namespace cpp_hist