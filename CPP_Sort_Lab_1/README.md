# ЛР1 — сортировки

## Вариант

Массив объектов: лотерейные билеты.

Поля:

- `ticketNumber` — номер билета;
- `nominalValue` — номинальная стоимость;
- `lotteryDate` — дата лотереи;
- `winAmount` — сумма выигрыша.

Правило сравнения реализовано в [src/lottery_ticket.h](src/lottery_ticket.h):

1. дата лотереи — по возрастанию;
2. выигрыш — по убыванию;
3. номер билета — по возрастанию.

Перегружены операторы `<`, `>`, `<=`, `>=`, `==`, `!=`.

## Реализованные алгоритмы

| Алгоритм | Файл |
|---|---|
| Сортировка выбором | [src/sortings.h](src/sortings.h) |
| Сортировка пузырьком | [src/sortings.h](src/sortings.h) |
| Пирамидальная сортировка | [src/sortings.h](src/sortings.h) |
| `std::sort` для сравнения | [src/main.cpp](src/main.cpp) |

## Запуск

```cmd
cd LR1
build.bat

python scripts\generate_data.py

build\lr1.exe

build\lr1.exe bench results\benchmark.csv ^
    data\lottery_100.csv data\lottery_250.csv data\lottery_500.csv ^
    data\lottery_1000.csv data\lottery_2000.csv data\lottery_4000.csv ^
    data\lottery_6000.csv data\lottery_8000.csv data\lottery_10000.csv ^
    data\lottery_15000.csv data\lottery_20000.csv data\lottery_30000.csv ^
    data\lottery_50000.csv data\lottery_70000.csv data\lottery_100000.csv

python scripts\plot_results.py results\benchmark.csv results\plot_linear.png
python scripts\plot_results.py results\benchmark.csv results\plot_loglog.png --log
```

## Результаты

Итоговые файлы:

- [results/benchmark.csv](results/benchmark.csv) — 15 наборов данных от 100 до 100000;
- [results/plot_linear.png](results/plot_linear.png) — общий график;
- [results/plot_loglog.png](results/plot_loglog.png) — график в log-log;
- [results/sorted_selection.csv](results/sorted_selection.csv);
- [results/sorted_bubble.csv](results/sorted_bubble.csv);
- [results/sorted_heap.csv](results/sorted_heap.csv);
- [results/sorted_std.csv](results/sorted_std.csv).

Все четыре отсортированных файла проверены: 100000 строк данных, порядок
соответствует правилу варианта, результаты всех алгоритмов совпадают.

На `100000` записей:

| Алгоритм | Время |
|---|---:|
| Selection sort | `13.7154` с |
| Bubble sort | `31.3492` с |
| Heap sort | `0.0219027` с |
| `std::sort` | `0.0147317` с |

## Вывод

Сортировка выбором и пузырьком имеют квадратичную сложность, поэтому на больших
наборах быстро становятся непрактичными. Пирамидальная сортировка и `std::sort`
ведут себя как `O(n log n)` и подходят для больших массивов. В практической
задаче лучше использовать `std::sort`; если требуется показать собственную
реализацию с гарантированной асимптотикой, подходит heap sort.

## Документация

- HTML-документация: [docs/html/index.html](docs/html/index.html)
- Ссылка на GitHub:
- Конфигурация для Doxygen

@page lottery_ticket_h lottery_ticket.h — Модель лотерейного билета

@section lottery_detailed Подробный разбор

Сначала идут стандартные include guards через `#pragma once` и подключение необходимых заголовков.
`<iosfwd>` подключает только предварительные объявления потоков, что ускоряет компиляцию.

Объявляем структуру `LotteryTicket`:
@code
struct LotteryTicket 
{
    std::string ticketNumber;  // Уникальный номер билета, строка чтобы сохранять ведущие нули (00000001)
    int nominalValue{};        // Номинал, инициализирован нулём через {}
    std::int32_t lotteryDate{};// Дата в формате YYYYMMDD (20220101). int32_t для переносимости размера
    long long winAmount{};     // Выигрыш в копейках/центах. 0 означает билет без выигрыша
    // ...
};
@endcode

`LotteryTicket() = default;` — просим компилятор сгенерировать конструктор по умолчанию, 
который инициализирует все поля их default-значениями.

Параметризованный конструктор:
@code
LotteryTicket(std::string t, int n, std::int32_t d, long long w)
    : ticketNumber(std::move(t)), nominalValue(n), lotteryDate(d), winAmount(w) {}
@endcode
Здесь используется список инициализации. 
`std::move(t)` перемещает строку внутрь поля, забирая её внутренний буфер без копирования. 

<b>Ключевая функция сравнения:</b>
@code
inline int compareTickets(const LotteryTicket& a, const LotteryTicket& b) noexcept
{
    if (a.lotteryDate != b.lotteryDate)
        return (a.lotteryDate < b.lotteryDate) ? -1 : 1;
    if (a.winAmount != b.winAmount)
        return (a.winAmount > b.winAmount) ? -1 : 1;
    if (a.ticketNumber != b.ticketNumber)
        return (a.ticketNumber < b.ticketNumber) ? -1 : 1;
    return 0;
}
@endcode

Далее идут операторы сравнения:
@code
inline bool operator< (const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) <  0; }
inline bool operator==(const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) == 0; }
// ... остальные аналогично
@endcode
Все операторы — `inline`, чтобы их код встраивался в место вызова без накладных расходов на функцию. 
Они просто вызывают `compareTickets` и проверяют возвращаемое значение. 

@page data_gen_h data_gen.h — Генератор тестовых данных

@section gen_detailed Подробный разбор

Генератор использует `<random>` для качественной случайности, 
а фиксированный seed делает последовательность воспроизводимой.

Функция `dateFromDayOffset`:
@code
inline std::int32_t dateFromDayOffset(int days)
{
    std::tm tm{};                        // Обнулённая структура tm
    tm.tm_year = 2022 - 1900;            // Год: 122 (отсчёт лет от 1900)
    tm.tm_mon = 0;                       // Месяц: 0 = январь
    tm.tm_mday = 1 + days;               // День: 1 + смещение в днях
    std::mktime(&tm);                    // Нормализует дату
    return (tm.tm_year + 1900) * 10000   // Год * 10000
         + (tm.tm_mon + 1) * 100         // Месяц * 100
         + tm.tm_mday;                   // День
}
@endcode

Функция `synthesize`:
@code
inline std::vector<LotteryTicket> synthesize(std::size_t count, std::uint32_t seed = 42)
{
    std::mt19937 rng(seed);              // Инициализация генератора зерном
    static constexpr int kNominals[] = { 50, 100, 150, 200, 500 };
    static constexpr int kDaysRange = 365 * 4;      // 4 года
    static constexpr int kWinProbPpm = 100;         // 100/1000 = 10% шанс выигрыша

    std::vector<LotteryTicket> out;
    out.reserve(count);                  // Сразу выделяем память под все элементы

    for (std::size_t i = 0; i < count; ++i)
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%08zu", i + 1); // Форматируем номер: "00000001" и т.д.
        
        const int nominal = kNominals[rng() % 5];        // Случайный выбор номинала
        const int day = static_cast<int>(rng() % (kDaysRange + 1)); // Случайный день в диапазоне
        const std::int32_t date = dateFromDayOffset(day); // Преобразуем смещение в реальную дату
        
        long long win = 0;                               // По умолчанию выигрыша нет
        if ((rng() % 1000) < kWinProbPpm)                // 10% шанс
        {
            switch (rng() % 5)                           // Равновероятный выбор типа выигрыша
            {
                case 0: win = nominal; break;            // Возврат номинала
                case 1: win = static_cast<long long>(nominal) * 5; break;   // x5
                case 2: win = static_cast<long long>(nominal) * 10; break;  // x10
                case 3: win = static_cast<long long>(nominal) * 100; break; // x100
                case 4: win = 100000; break;              // Фиксированный суперприз
            }
        }
        
        // Конструируем билет прямо в памяти вектора (без лишних копирований)
        out.emplace_back(std::string(buf), nominal, date, win);
    }
    return out;
}
@endcode

@page io_h io.h / io.cpp — Подсистема ввода-вывода CSV

@section io_detailed Подробный разбор

<b>parseDate — парсинг строки в дату:</b>
@code
std::int32_t parseDate(const std::string& s)
{
    // Жёсткая проверка формата: 10 символов, дефисы на позициях 4 и 7
    if (s.size() != 10 || s[4] != '-' || s[7] != '-')
        throw std::runtime_error("bad date format: " + s);
    
    int y = std::stoi(s.substr(0, 4));   // Извлекаем год (символы 0-3)
    int m = std::stoi(s.substr(5, 2));   // Извлекаем месяц (символы 5-6)
    int d = std::stoi(s.substr(8, 2));   // Извлекаем день (символы 8-9)
    
    return y * 10000 + m * 100 + d;      // Собираем в одно число
}
@endcode

<b>formatDate — обратное преобразование:</b>
@code
std::string formatDate(std::int32_t d)
{
    int year = d / 10000;               // Целочисленное деление даёт год 
    int month = (d / 100) % 100;        
    int day = d % 100;                  
    
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    return std::string(buf);            // Конструируем строку из буфера
}
@endcode

<b>split — разделение строки CSV:</b>
@code
static std::vector<std::string> split(const std::string& s, char sep = ',')
{
    std::vector<std::string> out;
    std::string cur;
    
    for (char c : s)
    {
        if (c == sep)                   // Встретили разделитель
        {
            out.push_back(cur);         // Сохраняем накопленное
            cur.clear();                // Очищаем для следующей колонки
        }
        else if (c != '\r')             // Игнорируем \r (Windows окончания строк)
        {
            cur.push_back(c);           // Добавляем символ к текущей колонке
        }
    }
    out.push_back(cur);                 // Добавляем последнюю колонку
    return out;
}
@endcode

<b>readCsv — чтение файла:</b>
@code
std::vector<LotteryTicket> readCsv(const std::string& path)
{
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open: " + path);  // Проверка открытия
    
    std::vector<LotteryTicket> out;
    std::string line;
    
    if (!std::getline(in, line)) return out;  // Читаем и пропускаем заголовок
    
    out.reserve(1 << 17);                     // Предварительное выделение
    
    while (std::getline(in, line))
    {
        if (line.empty()) continue;           // Пропускаем пустые строки
        
        auto cols = split(line, ',');
        if (cols.size() < 4) continue;        // Битые строки пропускаем
        
        LotteryTicket t;
        t.ticketNumber = cols[0];             // Копируем номер
        t.nominalValue = std::stoi(cols[1]);  // Парсим целое
        t.lotteryDate = parseDate(cols[2]);   // Парсим дату
        t.winAmount = std::stoll(cols[3]);    // Парсим long long
        
        out.push_back(std::move(t));          // Перемещаем
    }
    return out;
}
@endcode

<b>writeCsv — запись в файл:</b>
@code
void writeCsv(const std::string& path, const std::vector<LotteryTicket>& items) 
{
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write: " + path);
    
    out << "ticketNumber,nominalValue,lotteryDate,winAmount\n";  // Заголовок
    
    for (const auto& t : items) 
    {
        out << t.ticketNumber << ','
            << t.nominalValue << ','
            << formatDate(t.lotteryDate) << ','   // Конвертируем дату обратно в строку
            << t.winAmount << '\n';
    }
}
@endcode

@page sortings_h sortings.h — Алгоритмы сортировки

@section sortings_detailed Подробный разбор

<b>Сортировка выбором (Selection Sort):</b>
@code
template <class T>
void selectionSort(std::vector<T>& a)
{
    const std::size_t n = a.size();
    for (std::size_t i = 0; i + 1 < n; ++i)        // i — граница отсортированной части
    {
        std::size_t minIdx = i;                     // Предполагаем, что минимум на позиции i
        for (std::size_t j = i + 1; j < n; ++j)     // Ищем реальный минимум в правой части
            if (a[j] < a[minIdx])                   // Используем operator< из lottery_ticket.h!
                minIdx = j;
        if (minIdx != i)                            
            std::swap(a[i], a[minIdx]);             // Стандартная функция обмена
    }
}
@endcode

<b>Сортировка пузырьком (Bubble Sort):</b>
@code
template <class T>
void bubbleSort(std::vector<T>& a)
{
    const std::size_t n = a.size();
    if (n < 2) return;                              // Нечего сортировать

    for (std::size_t i = 0; i + 1 < n; ++i)         // Количество проходов
    {
        bool swapped = false;                       // Флаг: были ли обмены
        for (std::size_t j = 0; j + 1 < n - i; ++j) // n-i: правые элементы уже на месте
        {
            if (a[j] > a[j + 1])                    // Нарушение порядка
            {
                std::swap(a[j], a[j + 1]);          // Меняем местами
                swapped = true;                     // Был обмен
            }
        }
        if (!swapped) break;                        // Если обменов не было — массив отсортирован
    }
}
@endcode

<b>Пирамидальная сортировка (Heap Sort):</b>
Вспомогательная функция `siftDown` просеивает элемент вниз по куче:
@code
template <class T>
void siftDown(std::vector<T>& a, std::size_t root, std::size_t heapSize) 
{
    while (true)
    {
        std::size_t left = 2 * root + 1;            // Левый потомок (индексация с 0)
        std::size_t right = 2 * root + 2;           // Правый потомок
        std::size_t largest = root;                 // Считаем корень наибольшим
        
        if (left  < heapSize && a[left]  > a[largest]) largest = left;
        if (right < heapSize && a[right] > a[largest]) largest = right;
        // Здесь используется operator> из lottery_ticket.h!
        
        if (largest == root) break;                 // Свойство кучи восстановлено
        
        std::swap(a[root], a[largest]);             // Меняем с наибольшим потомком
        root = largest;                             // Продолжаем просеивание с новой позиции
    }
}
@endcode

Основная функция heapSort:
@code
template <class T>
void heapSort(std::vector<T>& a) 
{
    const std::size_t n = a.size();
    if (n < 2) return;
    
    // Фаза 1: Построение кучи снизу вверх
    for (std::size_t i = n / 2; i-- > 0; )         // i-- > 0 — идиоматический способ с беззнаковым счётчиком
        detail::siftDown(a, i, n);                  // Просеиваем каждый нетерминальный узел
    
    // Фаза 2: Сортировка
    for (std::size_t end = n - 1; end > 0; --end) 
    {
        std::swap(a[0], a[end]);                    // Максимум в конец неотсортированной части
        detail::siftDown(a, 0, end);                // Восстанавливаем кучу для уменьшенного размера
    }
}
@endcode

@page main_cpp main.cpp — Ядро программы и бенчмаркинг

@section main_detailed Подробный разбор

<b>Замер времени:</b>
@code
template <class Sorter>
static double timeOnce(std::vector<LotteryTicket>& a, Sorter sortFn) 
{
    auto t0 = clk::now();                              // Фиксируем время до
    sortFn(a);                                         // Выполняем сортировку 
    auto t1 = clk::now();                              // Фиксируем время после
    return std::chrono::duration<double>(t1 - t0).count(); // Разница в секундах
}
@endcode

<b>Обёртка для std::sort:</b>
@code
static void sortByStd(std::vector<LotteryTicket>& v) 
{
    std::sort(v.begin(), v.end());                     // Стандартная сортировка
}
@endcode

<b>runDefault — основной режим:</b>
@code
static int runDefault() 
{
    // ... создание директорий ...
    
    // Попытка чтения CSV, если не найден — генерация
    if (std::ifstream(inPath).good()) 
    {
        src = io::readCsv(inPath);                     // Читаем готовый файл
    } 
    else 
    {
        src = gen::synthesize(kDefaultSize);            // Генерируем 100000 записей
        io::writeCsv(inPath, src);                     // Сохраняем для будущих запусков
    }
    
    // Для каждой сортировки создаём КОПИЮ исходных данных
    auto a1 = src;  // Полное копирование вектора
    double t1 = timeOnce(a1, [](auto& v){ sortings::selectionSort(v); });
    // ... аналогично для bubble, heap, std::sort ...
    
    // Проверка, что все сортировки дали одинаковый результат
    bool ok = (a1 == a4) && (a2 == a4) && (a3 == a4);
    // operator== для векторов сравнивает все элементы через operator== LotteryTicket
    
    return ok ? 0 : 1;
}
@endcode

<b>main — точка входа:</b>
@code
int main(int argc, char** argv) 
{
    try 
    {
        if (argc >= 2 && std::string(argv[1]) == "bench")  // Проверяем режим
            return runBench(argc, argv);                    // Режим бенчмаркинга
        return runDefault();                                // Обычный режим
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";        // Исключения
        return 10;                                         // Код ошибки
    }
}
@endcode

*/


 
