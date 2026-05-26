\mainpage ЛР2 — поиск всех вхождений по ключу

\section variant Вариант

Структура данных взята из ЛР1: лотерейный билет
`ticketNumber, nominalValue, lotteryDate, winAmount`.

Ключ поиска по заданию — первое нечисловое поле объекта. В этой работе ключом
считается `lotteryDate` в формате `YYYY-MM-DD`.

\section methods Реализованные методы

| Метод | Реализация | Особенность |
|---|---|---|
| Линейный поиск | \ref linear_search.h | Полный просмотр массива, `O(n)` |
| Бинарное дерево поиска | \ref bst.h | Узел хранит список всех объектов с одинаковой датой |
| Красно-чёрное дерево | \ref rb_tree.h | Самостоятельная балансировка после вставки |
| Хэш-таблица | \ref hash_table.h | Метод цепочек, FNV-1a, подсчёт коллизий |
| `std::multimap` | \ref main.cpp | Добавлен для сравнения по требованию задания |

\section results Результаты

На \c 1'000'000 записей получено:

| Метод | Среднее время поиска |
|---|---:|
| Линейный | `0.008760737` с |
| BST | `0.000000951` с |
| RB-tree | `0.000000755` с |
| Hash table | `0.000000558` с |
| `std::multimap` | `0.000075411` с |

\section conclusion Вывод

Линейный поиск подходит только для маленьких массивов или разовых запросов.
Если запросов много, выгодно один раз построить индекс. Лучшее время поиска в
наших данных дала самописная хэш-таблица. Красно-чёрное дерево немного медленнее,
но даёт гарантированную высоту `O(n)`. Обычное BST на случайных данных
работает приемлемо, но в худшем случае может выродиться. `std::multimap`
уступает хэш-таблице по скорости поиска и построения, но удобен как готовый
ассоциативный контейнер.

\page lottery_ticket.h LotteryTicket — структура данных лотерейного билета
\brief Определение основной структуры данных

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include <string>                                  // Для std::string (номера и даты)
#include <iosfwd>                                  // Forward declaration для потоков
#include <cstdint>                                 // Для uint64_t (при необходимости)

struct LotteryTicket 
{
    std::string ticketNumber;                      // Номер билета (уникальный идентификатор)
    int         nominalValue{};                    // Номинал в рублях, {} для нуля по умолчанию
    std::string lotteryDate;                       // Дата розыгрыша YYYY-MM-DD (ключ поиска!)
    long long   winAmount{};                       // Выигрыш в копейках (избегаем float)

    LotteryTicket() = default;                     // Конструктор по умолчанию
    LotteryTicket(std::string t, int n, std::string d, long long w)
        : ticketNumber(std::move(t))               // move-семантика для эффективности
        , nominalValue(n)                          // прямой copy int'а
        , lotteryDate(std::move(d))                // move для строки
        , winAmount(w) {}                          // прямой copy long long'а

    bool operator==(const LotteryTicket& o) const noexcept  // Сравнение для тестов
    {
        return ticketNumber == o.ticketNumber      // Сравниваем номер
            && nominalValue == o.nominalValue      // Сравниваем номинал
            && lotteryDate == o.lotteryDate        // Сравниваем дату
            && winAmount == o.winAmount;           // Сравниваем выигрыш
    }
};
\endcode

\page linear_search.h Linear Search — линейный поиск
\brief Простейший алгоритм поиска перебором

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include "lottery_ticket.h"                        // Наша структура билета
#include <string>                                  // Для std::string (ключа поиска)
#include <vector>                                  // Для возврата результатов

namespace search                                    // Все алгоритмы в пространстве имён search
{

    inline std::vector<const LotteryTicket*>       // inline для определения в заголовке
        linearFindAll(const std::vector<LotteryTicket>& data, const std::string& key) 
    {
        std::vector<const LotteryTicket*> out;     // Вектор указателей (без копирования)
        for (const auto& t : data)                 // Проходим по всем билетам
            if (t.lotteryDate == key)              // Если дата совпадает с ключом
                out.push_back(&t);                 // Добавляем указатель в результат
        return out;                                // Возвращаем все найденные билеты
    }

}
\endcode

\page bst.h BST — бинарное дерево поиска
\brief Реализация BST с группировкой по ключу (дате)

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include "lottery_ticket.h"                        // Структура билета
#include <vector>                                  // Для bucket'ов (векторов указателей)
#include <string>                                  // Для ключа-строки

namespace search                                    // Пространство имён для алгоритмов
{

    class BST
    {
    public:
        BST() = default;                           // Корень инициализируется nullptr
        BST(const BST&) = delete;                  // Запрет копирования (уникальное владение)
        BST& operator=(const BST&) = delete;       // Запрет присваивания
        ~BST() { destroy(root_); }                 // Деструктор рекурсивно удаляет узлы

        void build(const std::vector<LotteryTicket>& data)  // Построение дерева из вектора
        {
            destroy(root_);                        // Удаляем старое дерево, если было
            root_ = nullptr;                      // Обнуляем корень
            for (const auto& t : data)            // Для каждого билета
                insert(&t);                       // Вставляем указатель в дерево
        }

        void insert(const LotteryTicket* p)        // Вставка билета (по указателю)
        {
            Node** cur = &root_;                   // Двойной указатель для удобной замены
            while (*cur) {                         // Пока не дошли до листа
                if (p->lotteryDate == (*cur)->key) // Если дата уже существует в узле
                {
                    (*cur)->bucket.push_back(p);   // Просто добавляем в bucket
                    return;                        // Выходим, новый узел не нужен
                }
                // Выбираем направление: влево (меньше) или вправо (больше)
                cur = (p->lotteryDate < (*cur)->key) ? &(*cur)->left : &(*cur)->right;
            }
            // Создаём новый узел с ключом и одним билетом в bucket'е
            *cur = new Node{ p->lotteryDate, {p}, nullptr, nullptr };
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const  // Поиск по ключу
        {
            const Node* cur = root_;               // Начинаем с корня
            while (cur) {                          // Пока не дошли до листа
                if (key == cur->key)               // Если ключ найден
                    return cur->bucket;            // Возвращаем все билеты этой даты
                // Идём влево или вправо в зависимости от сравнения
                cur = (key < cur->key) ? cur->left : cur->right;
            }
            return {};                             // Ключ не найден — пустой результат
        }

    private:
        struct Node                                // Внутренняя структура узла дерева
        {
            std::string                       key; // Ключ (дата розыгрыша)
            std::vector<const LotteryTicket*> bucket; // Вектор указателей на билеты
            Node* left;                        // Левый потомок
            Node* right;                       // Правый потомок
        };

        static void destroy(Node* n)               // Рекурсивное удаление узлов
        {
            if (!n) return;                        // Базовый случай: nullptr
            destroy(n->left);                      // Удаляем левое поддерево
            destroy(n->right);                     // Удаляем правое поддерево
            delete n;                              // Удаляем текущий узел
        }

        Node* root_ = nullptr;                     // Корень дерева (изначально пуст)
    };
}
\endcode

\page rb_tree.h RBTree — красно-чёрное дерево
\brief Самобалансирующееся дерево с гарантированной высотой O(log n)

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include "lottery_ticket.h"                        // Структура билета
#include <string>                                  // Для ключа
#include <vector>                                  // Для bucket'ов

namespace search
{

    class RBTree
    {
    public:
        RBTree()                                   // Конструктор инициализирует NIL-страж
        {
            nil_ = new Node{};                     // Создаём страж (листовой узел)
            nil_->color = Color::Black;            // NIL всегда чёрный
            nil_->left = nil_->right = nil_->parent = nil_;  // Все ссылки на себя
            root_ = nil_;                          // Корень указывает на NIL (дерево пусто)
        }

        RBTree(const RBTree&) = delete;            // Запрет копирования
        RBTree& operator=(const RBTree&) = delete; // Запрет присваивания

        ~RBTree() { destroy(root_); delete nil_; } // Удаляем дерево и страж

        void build(const std::vector<LotteryTicket>& data)  // Построение из вектора
        {
            destroy(root_);                        // Удаляем старое дерево
            root_ = nil_;                          // Корень указывает на NIL
            for (const auto& t : data)             // Для каждого билета
                insert(&t);                        // Вставляем в дерево
        }

        void insert(const LotteryTicket* p)        // Вставка с балансировкой
        {
            Node* parent = nil_;                   // Родитель текущего узла
            Node* cur = root_;                     // Текущий узел (начинаем с корня)
            while (cur != nil_) {                  // Пока не дошли до NIL
                parent = cur;                      // Запоминаем родителя
                if (p->lotteryDate == cur->key)    // Дата уже существует
                {
                    cur->bucket.push_back(p);      // Добавляем в существующий bucket
                    return;                        // Выходим (балансировка не нужна)
                }
                // Выбираем направление
                cur = (p->lotteryDate < cur->key) ? cur->left : cur->right;
            }

            // Создаём новый узел (всегда красный, кроме корня)
            Node* z = new Node{ p->lotteryDate, {p}, Color::Red, nil_, nil_, parent };
            if (parent == nil_)                    // Если дерево было пустым
                root_ = z;                         // Новый узел становится корнем
            else if (z->key < parent->key)         // Если меньше родителя
                parent->left = z;                  // Становимся левым ребёнком
            else                                   // Если больше родителя
                parent->right = z;                 // Становимся правым ребёнком
            fixInsert(z);                          // Восстанавливаем RB-свойства
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const  // Поиск
        {
            const Node* cur = root_;               // Начинаем с корня
            while (cur != nil_) {                  // Пока не дошли до NIL
                if (key == cur->key)               // Ключ найден
                    return cur->bucket;            // Возвращаем все билеты
                cur = (key < cur->key) ? cur->left : cur->right;  // Идём влево/вправо
            }
            return {};                             // Ключ не найден
        }

    private:
        enum class Color { Red, Black };           // Два цвета узлов

        struct Node                                // Структура узла с parent'ом
        {
            std::string                       key; // Ключ (дата)
            std::vector<const LotteryTicket*> bucket; // Билеты этой даты
            Color  color = Color::Black;           // Цвет узла
            Node* left = nullptr;                  // Левый потомок
            Node* right = nullptr;                 // Правый потомок
            Node* parent = nullptr;                // Родитель (нужен для балансировки)
        };

        Node* root_ = nullptr;                     // Корень дерева
        Node* nil_ = nullptr;                      // NIL-страж (все листья)

        void destroy(Node* n)                      // Рекурсивное удаление узлов
        {
            if (!n || n == nil_) return;           // Не удаляем NIL и nullptr
            destroy(n->left);                      // Удаляем левое поддерево
            destroy(n->right);                     // Удаляем правое поддерево
            delete n;                              // Удаляем текущий узел
        }

        void rotateLeft(Node* x)                   // Левый поворот вокруг x
        {
            Node* y = x->right;                    // y — правый ребёнок x
            x->right = y->left;                    // Поддерево y->left становится правым x
            if (y->left != nil_)                   // Если поддерево не пусто
                y->left->parent = x;               // Обновляем parent
            y->parent = x->parent;                 // y поднимается на место x
            if (x->parent == nil_)                 // Если x был корнем
                root_ = y;                         // y становится корнем
            else if (x == x->parent->left)         // Если x был левым ребёнком
                x->parent->left = y;               // y становится левым ребёнком родителя
            else                                   // Если x был правым ребёнком
                x->parent->right = y;              // y становится правым ребёнком
            y->left = x;                           // x становится левым ребёнком y
            x->parent = y;                         // Обновляем parent x
        }

        void rotateRight(Node* x)                  // Правый поворот вокруг x
        {
            Node* y = x->left;                     // y — левый ребёнок x
            x->left = y->right;                    // Поддерево y->right становится левым x
            if (y->right != nil_)                  // Если поддерево не пусто
                y->right->parent = x;              // Обновляем parent
            y->parent = x->parent;                 // y поднимается на место x
            if (x->parent == nil_)                 // Если x был корнем
                root_ = y;                         // y становится корнем
            else if (x == x->parent->right)        // Если x был правым ребёнком
                x->parent->right = y;              // y становится правым ребёнком
            else                                   // Если x был левым ребёнком
                x->parent->left = y;               // y становится левым ребёнком
            y->right = x;                          // x становится правым ребёнком y
            x->parent = y;                         // Обновляем parent x
        }

        void fixInsert(Node* z)                    // Восстановление RB-свойств после вставки
        {
            while (z->parent->color == Color::Red) // Пока родитель красный (нарушение)
            {
                if (z->parent == z->parent->parent->left)  // Родитель — левый ребёнок
                {
                    Node* uncle = z->parent->parent->right;  // Дядя (правый ребёнок деда)
                    if (uncle->color == Color::Red)          // Случай 1: дядя красный
                    {
                        z->parent->color = Color::Black;     // Перекрашиваем родителя
                        uncle->color = Color::Black;         // Перекрашиваем дядю
                        z->parent->parent->color = Color::Red; // Дед становится красным
                        z = z->parent->parent;               // Поднимаемся к деду
                    }
                    else                                     // Случай 2 и 3: дядя чёрный
                    {
                        if (z == z->parent->right)           // Случай 2: z — правый ребёнок
                        {
                            z = z->parent;                   // Поднимаемся к родителю
                            rotateLeft(z);                   // Левый поворот
                        }
                        // Случай 3: z — левый ребёнок
                        z->parent->color = Color::Black;     // Родитель становится чёрным
                        z->parent->parent->color = Color::Red; // Дед становится красным
                        rotateRight(z->parent->parent);      // Правый поворот вокруг деда
                    }
                }
                else                                         // Симметричный случай (родитель — правый ребёнок)
                {
                    Node* uncle = z->parent->parent->left;   // Дядя (левый ребёнок деда)
                    if (uncle->color == Color::Red)          // Случай 1: дядя красный
                    {
                        z->parent->color = Color::Black;
                        uncle->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        z = z->parent->parent;
                    }
                    else                                     // Случай 2 и 3: дядя чёрный
                    {
                        if (z == z->parent->left)            // Случай 2: z — левый ребёнок
                        {
                            z = z->parent;
                            rotateRight(z);                  // Правый поворот
                        }
                        // Случай 3: z — правый ребёнок
                        z->parent->color = Color::Black;
                        z->parent->parent->color = Color::Red;
                        rotateLeft(z->parent->parent);       // Левый поворот вокруг деда
                    }
                }
            }
            root_->color = Color::Black;             // Корень всегда чёрный
        }
    };
}
\endcode

\page hash_table.h HashTable — хэш-таблица
\brief Метод цепочек, FNV-1a хэш-функция, автоматический рехэшинг

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include "lottery_ticket.h"                        // Структура билета
#include <cstdint>                                 // Для uint64_t
#include <string>                                  // Для ключа
#include <vector>                                  // Для bucket'ов и таблицы

namespace search
{

    class HashTable
    {
    public:
        explicit HashTable(std::size_t initialBuckets = 16)  // Конструктор с размером
        {
            buckets_.resize(roundPow2(initialBuckets));  // Размер — степень двойки
            mask_ = buckets_.size() - 1;              // Маска для быстрого индекса (x & mask == x % size)
        }

        void build(const std::vector<LotteryTicket>& data)  // Построение из вектора
        {
            buckets_.assign(buckets_.size(), {});      // Очищаем все цепочки
            size_ = 0;                                 // Обнуляем счётчик уникальных ключей
            for (const auto& t : data)                 // Для каждого билета
                insert(&t);                            // Вставляем в таблицу
        }

        void insert(const LotteryTicket* p)            // Вставка билета
        {
            // Если загрузка > 75% (size_ * 4 >= buckets_.size() * 3), расширяем таблицу
            if (size_ * 4 >= buckets_.size() * 3)
                rehash(buckets_.size() * 2);           // Удваиваем размер

            std::uint64_t h = fnv1a(p->lotteryDate);   // Вычисляем хэш ключа
            std::size_t idx = static_cast<std::size_t>(h) & mask_;  // Индекс через битовую маску
            
            for (auto& e : buckets_[idx])              // Ищем ключ в цепочке
            {
                if (e.key == p->lotteryDate)           // Если ключ уже есть
                {
                    e.bucket.push_back(p);             // Добавляем в существующий bucket
                    return;                            // Выходим (новый Entry не нужен)
                }
            }
            // Новый ключ — создаём Entry с одним билетом в bucket'е
            buckets_[idx].push_back(Entry{ p->lotteryDate, {p} });
            ++size_;                                   // Увеличиваем счётчик ключей
        }

        std::vector<const LotteryTicket*> findAll(const std::string& key) const  // Поиск
        {
            std::uint64_t h = fnv1a(key);              // Вычисляем хэш ключа
            std::size_t idx = static_cast<std::size_t>(h) & mask_;  // Индекс в таблице
            for (const auto& e : buckets_[idx])        // Проходим по цепочке
                if (e.key == key)                      // Если ключ совпал
                    return e.bucket;                   // Возвращаем все билеты
            return {};                                 // Ключ не найден
        }

        // Методы для анализа производительности хэш-таблицы
        std::size_t uniqueKeys() const { return size_; }           // Количество уникальных ключей
        std::size_t bucketCount() const { return buckets_.size(); } // Количество bucket'ов

        std::size_t collisionCount() const               // Суммарное число коллизий
        {
            std::size_t collisions = 0;
            for (const auto& bucket : buckets_)          // Для каждого bucket'а
                if (bucket.size() > 1)                   // Если в цепочке >1 элемента
                    collisions += bucket.size() - 1;     // Коллизий = (размер - 1)
            return collisions;
        }

        std::size_t maxChainLength() const               // Максимальная длина цепочки
        {
            std::size_t best = 0;
            for (const auto& bucket : buckets_)
                if (bucket.size() > best) best = bucket.size();
            return best;
        }

        std::size_t nonEmptyBuckets() const              // Количество непустых bucket'ов
        {
            std::size_t count = 0;
            for (const auto& bucket : buckets_)
                if (!bucket.empty()) ++count;
            return count;
        }

    private:
        struct Entry                                     // Одна запись в bucket'е
        {
            std::string                       key;      // Ключ (дата)
            std::vector<const LotteryTicket*> bucket;   // Билеты этой даты
        };

        static std::size_t roundPow2(std::size_t v)      // Округление до степени двойки
        {
            std::size_t p = 1;
            while (p < v) p <<= 1;                       // Удваиваем, пока не >= v
            return p;
        }

        static std::uint64_t fnv1a(const std::string& s) // FNV-1a хэш-функция
        {
            std::uint64_t h = 1469598103934665603ull;    // Offsets basis (FNV-1a)
            for (unsigned char c : s) {                  // Для каждого байта строки
                h ^= c;                                  // XOR с текущим байтом
                h *= 1099511628211ull;                   // Умножение на простое число FNV-prime
            }
            return h;
        }

        void rehash(std::size_t newCount)                // Перестройка таблицы (при росте)
        {
            std::vector<std::vector<Entry>> old;         // Сохраняем старую таблицу
            old.swap(buckets_);                          // Быстрый обмен без копирования
            buckets_.assign(newCount, {});               // Создаём новую таблицу
            mask_ = newCount - 1;                        // Новая маска для индексации
            
            for (auto& bucket : old)                     // Для каждого старого bucket'а
                for (auto& e : bucket)                   // Для каждого Entry в нём
                {
                    std::uint64_t h = fnv1a(e.key);      // Пересчитываем хэш
                    std::size_t idx = static_cast<std::size_t>(h) & mask_;  // Новый индекс
                    buckets_[idx].push_back(std::move(e));  // Перемещаем Entry (без копирования)
                }
        }

        std::vector<std::vector<Entry>> buckets_;       // Таблица: массив списков Entry
        std::size_t                     mask_ = 0;      // Маска = size-1 для быстрого индекса
        std::size_t                     size_ = 0;      // Количество уникальных ключей
    };
}
\endcode

\page io.h io — ввод/вывод данных
\brief Чтение CSV-файлов и запись результатов поиска

\code{.cpp}
#pragma once                                       // Защита от множественного включения

#include "lottery_ticket.h"                        // Структура билета
#include <vector>                                  // Для возврата вектора билетов
#include <string>                                  // Для путей и ключей

namespace io
{
    // Читает CSV-файл с билетами, возвращает вектор структур
    std::vector<LotteryTicket> readCsv(const std::string& path);
    
    // Записывает найденные билеты в CSV-файл с метаданными (# method, key, matches)
    void writeFound(const std::string& path, const std::string& method, 
                    const std::string& key, const std::vector<const LotteryTicket*>& results);
}

// Реализация в io.cpp
\endcode

\page io.cpp io.cpp — реализация ввода/вывода
\brief Внутренние детали парсинга CSV

\code{.cpp}
#include "io.h"                                    // Заголовок с объявлениями

#include <fstream>                                 // Для std::ifstream, std::ofstream
#include <sstream>                                 // Для строковых потоков
#include <stdexcept>                               // Для std::runtime_error

namespace io
{

    // Вспомогательная функция: разбивает строку по разделителю (по умолчанию ',')
    static std::vector<std::string> split(const std::string& s, char sep = ',')
    {
        std::vector<std::string> out;              // Результат разбиения
        std::string cur;                           // Текущая накапливаемая подстрока
        for (char c : s) {                         // Проходим по всем символам
            if (c == sep) {                        // Если встретили разделитель
                out.push_back(cur);                // Добавляем накопленное
                cur.clear();                       // Очищаем для следующего поля
            }
            else if (c != '\r')                    // Игнорируем символ \r (Windows-формат)
                cur.push_back(c);                  // Иначе добавляем символ в текущее поле
        }
        out.push_back(cur);                        // Добавляем последнее поле
        return out;
    }

    std::vector<LotteryTicket> readCsv(const std::string& path)  // Чтение CSV-файла
    {
        std::ifstream in(path);                    // Открываем файл
        if (!in)                                   // Если не открылся
            throw std::runtime_error("cannot open: " + path);
        
        std::vector<LotteryTicket> out;            // Результирующий вектор
        out.reserve(1 << 17);                      // Предвыделение на 131072 билета (оптимизация)
        
        std::string line;                          // Буфер для строки
        if (!std::getline(in, line))               // Пропускаем заголовок (первую строку)
            return out;                            // Файл пуст — возвращаем пустой вектор
        
        while (std::getline(in, line))             // Читаем строки данных
        {
            if (line.empty()) continue;            // Пропускаем пустые строки
            auto c = split(line, ',');             // Разбиваем на поля
            if (c.size() < 4) continue;            // Если полей меньше 4 — битый CSV, пропускаем
            
            // Создаём билет: номер, номинал, дата, выигрыш
            out.emplace_back(c[0], std::stoi(c[1]), c[2], std::stoll(c[3]));
        }
        return out;                                // Возвращаем прочитанные билеты
    }

    void writeFound(const std::string& path, const std::string& method, 
                    const std::string& key, const std::vector<const LotteryTicket*>& results)
    {
        std::ofstream out(path);                   // Открываем файл для записи
        if (!out)                                  // Если не создался
            throw std::runtime_error("cannot write: " + path);
        
        // Записываем метаданные в комментарий CSV (начинается с #)
        out << "# method=" << method << " key=" << key
            << " matches=" << results.size() << "\n";
        out << "ticketNumber,nominalValue,lotteryDate,winAmount\n";  // Заголовок CSV
        
        for (const auto* t : results) {            // Для каждого найденного билета
            out << t->ticketNumber << ','          // Номер
                << t->nominalValue << ','          // Номинал
                << t->lotteryDate << ','           // Дата
                << t->winAmount << '\n';           // Выигрыш
        }
    }
}
\endcode

\page main.cpp Main — точка входа и бенчмаркинг
\brief Демонстрация работы и измерение производительности всех алгоритмов

\code{.cpp}
#include "lottery_ticket.h"                        // Структура билета
#include "io.h"                                    // Чтение/запись CSV
#include "linear_search.h"                         // Линейный поиск
#include "bst.h"                                   // Бинарное дерево
#include "rb_tree.h"                               // Красно-чёрное дерево
#include "hash_table.h"                            // Хэш-таблица

#include <algorithm>                               // std::sort, std::shuffle
#include <chrono>                                  // Измерение времени
#include <cstdio>                                  // std::fprintf, std::snprintf
#include <iostream>                                // std::cout, std::cerr
#include <iterator>                                // std::distance
#include <map>                                     // std::multimap (эталон)
#include <random>                                  // std::mt19937_64
#include <set>                                     // std::set для уникальных ключей
#include <string>                                  // std::string
#include <vector>                                  // std::vector

using clk = std::chrono::high_resolution_clock;    // Часы с максимальной точностью
using sec = std::chrono::duration<double>;         // Duration в секундах (double)

static volatile std::size_t gBenchmarkSink = 0;    // volatile запрещает оптимизатору выкидывать вызовы

// Выбирает случайные ключи для тестовых запросов (n штук, seed для воспроизводимости)
static std::vector<std::string> sampleKeys(const std::vector<LotteryTicket>& data,
                                           std::size_t n, std::uint64_t seed) 
{
    std::set<std::string> uniq;                    // set для уникальных ключей
    for (const auto& t : data)                     // Проходим по всем билетам
        uniq.insert(t.lotteryDate);                // Добавляем дату в set (автоматически сортирует)
    
    std::vector<std::string> all(uniq.begin(), uniq.end());  // Все уникальные даты
    
    std::mt19937_64 rng(seed);                     // Генератор случайных чисел
    std::shuffle(all.begin(), all.end(), rng);     // Перемешиваем
    
    if (all.size() > n) all.resize(n);             // Оставляем только n ключей
    return all;
}

// Проверяет, что два вектора указателей содержат одни и те же билеты (порядок не важен)
static bool sameResults(std::vector<const LotteryTicket*> a, 
                        std::vector<const LotteryTicket*> b) 
{
    if (a.size() != b.size()) return false;        // Разный размер — уже не совпадают
    
    auto cmp = [](const LotteryTicket* x, const LotteryTicket* y) { 
        return x < y;                              // Сравнение по адресу (уникально)
    };
    std::sort(a.begin(), a.end(), cmp);            // Сортируем оба вектора
    std::sort(b.begin(), b.end(), cmp);            // (порядок не важен, но для сравнения нужен)
    return a == b;                                 // Поэлементное сравнение после сортировки
}

using MultiMapIndex = std::multimap<std::string, LotteryTicket>;  // Тип эталонного контейнера

// Построение multimap (хранит копии объектов, не указатели)
static void buildMultiMap(const std::vector<LotteryTicket>& data, MultiMapIndex& index) 
{
    index.clear();                                 // Очищаем старый контейнер
    for (const auto& t : data)                     // Для каждого билета
        index.emplace(t.lotteryDate, t);           // Вставляем копию с ключом = дате
}

// Подсчёт количества билетов с заданным ключом в multimap
static std::size_t multimapCount(const MultiMapIndex& index, const std::string& key) 
{
    auto range = index.equal_range(key);           // Получаем итераторы на диапазон
    return static_cast<std::size_t>(std::distance(range.first, range.second));  // Расстояние = количество
}

// Демонстрационный режим: загружает 100000 билетов и показывает работу всех методов
static int runDemo() 
{
    const std::string inPath = "data/lottery_100000.csv";
    std::cout << "[LR2] reading " << inPath << " ...\n";
    auto data = io::readCsv(inPath);
    std::cout << "[LR2] loaded " << data.size() << " tickets\n";

    auto t0 = clk::now();                          // Засекаем время до построения BST
    search::BST       bst;     bst.build(data);    // Строим BST
    auto t1 = clk::now();                          // Время после BST
    search::RBTree    rb;      rb.build(data);     // Строим RB-дерево
    auto t2 = clk::now();                          // Время после RB
    search::HashTable ht;      ht.build(data);     // Строим хэш-таблицу
    auto t3 = clk::now();                          // Время после хэш-таблицы
    MultiMapIndex mm;          buildMultiMap(data, mm);  // Строим multimap
    auto t4 = clk::now();                          // Время после multimap

    // Выводим время построения
    std::cout << "  build BST       : " << sec(t1 - t0).count() << " s\n";
    std::cout << "  build RB-tree   : " << sec(t2 - t1).count() << " s\n";
    std::cout << "  build HashTable : " << sec(t3 - t2).count() << " s\n";
    std::cout << "  build multimap  : " << sec(t4 - t3).count() << " s\n";
    
    // Выводим статистику хэш-таблицы
    std::cout << "  hash collisions : " << ht.collisionCount()
              << " (buckets=" << ht.bucketCount()
              << ", max_chain=" << ht.maxChainLength() << ")\n";

    auto keys = sampleKeys(data, 5, 1);            // Берём 5 случайных ключей

    for (const auto& key : keys)                   // Для каждого ключа
    {
        std::cout << "\n--- key = " << key << " ---\n";
        auto rLin  = search::linearFindAll(data, key);  // Линейный поиск
        auto rBst  = bst.findAll(key);                   // Поиск в BST
        auto rRb   = rb.findAll(key);                    // Поиск в RB-дереве
        auto rHash = ht.findAll(key);                    // Поиск в хэш-таблице
        auto rMap  = multimapCount(mm, key);             // Подсчёт в multimap

        std::cout << "  linear : " << rLin.size()  << " matches\n";
        std::cout << "  BST    : " << rBst.size()  << " matches\n";
        std::cout << "  RB     : " << rRb.size()   << " matches\n";
        std::cout << "  hash   : " << rHash.size() << " matches\n";
        std::cout << "  mmap   : " << rMap         << " matches\n";

        // Проверяем, что все методы вернули одинаковые результаты
        bool ok = sameResults(rLin, rBst)
               && sameResults(rLin, rRb)
               && sameResults(rLin, rHash)
               && (rLin.size() == rMap);
        std::cout << "  consistency: " << (ok ? "OK" : "FAIL") << "\n";

        // Сохраняем результаты линейного поиска в CSV (для проверки)
        std::string safeKey = key;
        for (auto& c : safeKey) if (c == ':' || c == '/') c = '_';  // Заменяем недопустимые символы
        io::writeFound("results/found_" + safeKey + ".csv", "linear", key, rLin);
    }

    return 0;
}

// Бенчмарк-режим: измеряет время построения и поиска на разных размерах данных
static int runBench(int argc, char** argv) 
{
    if (argc < 4) {
        std::cerr << "usage: bench out.csv in1.csv [in2.csv ...]\n";
        return 2;
    }

    const std::string outPath = argv[2];           // Путь к выходному CSV

    FILE* fp = std::fopen(outPath.c_str(), "w");   // Открываем CSV для записи
    if (!fp) { std::perror("fopen"); return 3; }
    
    // Заголовок CSV со всеми метриками
    std::fprintf(fp,
        "size,n_queries,build_bst,build_rb,build_hash,build_multimap,"
        "hash_buckets,hash_non_empty,hash_collisions,hash_max_chain,"
        "search_linear_avg,search_bst_avg,search_rb_avg,search_hash_avg,"
        "search_multimap_avg\n");

    constexpr std::size_t kQueries = 100;          // Делаем 100 запросов для каждого размера

    for (int i = 3; i < argc; ++i)                 // Для каждого входного файла
    {
        const std::string in = argv[i];
        std::cout << "[bench] " << in << " ...\n";
        auto data = io::readCsv(in);               // Читаем данные
        const std::size_t n = data.size();

        // Измеряем время построения четырёх структур данных
        auto t0 = clk::now();
        search::BST bst;        bst.build(data);
        auto t1 = clk::now();
        search::RBTree rb;      rb.build(data);
        auto t2 = clk::now();
        search::HashTable ht;   ht.build(data);
        auto t3 = clk::now();
        MultiMapIndex mm;       buildMultiMap(data, mm);
        auto t4 = clk::now();

        const double tBuildBst  = sec(t1 - t0).count();   // Время построения BST
        const double tBuildRb   = sec(t2 - t1).count();   // Время построения RB-дерева
        const double tBuildHash = sec(t3 - t2).count();   // Время построения хэш-таблицы
        const double tBuildMap  = sec(t4 - t3).count();   // Время построения multimap

        // Выбираем kQueries/2 ключей из существующих дат
        auto hits = sampleKeys(data, kQueries / 2, 42 + n);
        std::vector<std::string> queries = hits;
        
        // Добавляем недостающие запросы (до kQueries) из вымышленных дат (не существующих)
        for (std::size_t k = 0; queries.size() < kQueries; ++k) 
        {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "1900-01-%02zu", (k % 28) + 1);
            queries.emplace_back(buf);             // Даты 1900-01-01...1900-01-28 (не существуют)
        }

        // Лямбда для измерения времени поиска (возвращает среднее время одного запроса)
        auto runQueries = [&](auto fn) 
        {
            auto a = clk::now();
            std::size_t total = 0;
            for (const auto& q : queries) total += fn(q).size();
            auto b = clk::now();
            gBenchmarkSink = total;                // volatile — чтобы оптимизатор не удалил вызовы
            return sec(b - a).count() / queries.size();  // Среднее время на запрос
        };

        double tLin  = runQueries([&](const std::string& q){ return search::linearFindAll(data, q); });
        double tBst  = runQueries([&](const std::string& q){ return bst.findAll(q); });
        double tRb   = runQueries([&](const std::string& q){ return rb.findAll(q); });
        double tHash = runQueries([&](const std::string& q){ return ht.findAll(q); });
        
        // Для multimap отдельно, т.к. он возвращает количество, а не вектор
        auto runCountQueries = [&](auto fn) 
        {
            auto a = clk::now();
            std::size_t total = 0;
            for (const auto& q : queries) total += fn(q);
            auto b = clk::now();
            gBenchmarkSink = total;
            return sec(b - a).count() / queries.size();
        };
        double tMap = runCountQueries([&](const std::string& q){ return multimapCount(mm, q); });

        // Выводим результаты в консоль
        std::cout << "  build  (B/R/H/M) = " << tBuildBst << " / " << tBuildRb
                  << " / " << tBuildHash << " / " << tBuildMap << " s\n";
        std::cout << "  hash collisions = " << ht.collisionCount()
                  << "  max_chain=" << ht.maxChainLength()
                  << "  buckets=" << ht.bucketCount() << "\n";
        std::cout << "  search avg       = " << tLin << " / " << tBst << " / "
                  << tRb << " / " << tHash << " / " << tMap << " s\n";

        // Записываем строку в CSV
        std::fprintf(fp,
            "%zu,%zu,%.9f,%.9f,%.9f,%.9f,%zu,%zu,%zu,%zu,"
            "%.9f,%.9f,%.9f,%.9f,%.9f\n",
            n, queries.size(), tBuildBst, tBuildRb, tBuildHash, tBuildMap,
            ht.bucketCount(), ht.nonEmptyBuckets(), ht.collisionCount(), ht.maxChainLength(),
            tLin, tBst, tRb, tHash, tMap);
        std::fflush(fp);
    }
    std::fclose(fp);
    return 0;
}

int main(int argc, char** argv) 
{
    try 
    {
        // Если первый аргумент "bench" — запускаем бенчмарк, иначе демо
        if (argc >= 2 && std::string(argv[1]) == "bench")
            return runBench(argc, argv);
        return runDemo();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 10;
    }
}
\endcode
