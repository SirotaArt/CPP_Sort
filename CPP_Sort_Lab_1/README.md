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
| Selection sort | `22.8042` с |
| Bubble sort | `71.0709` с |
| Heap sort | `0.0454` с |
| `std::sort` | `0.0309` с |

## Вывод

Сортировка выбором и пузырьком имеют квадратичную сложность, поэтому на больших
наборах быстро становятся непрактичными. Пирамидальная сортировка и `std::sort`
ведут себя как `O(n log n)` и подходят для больших массивов. В практической
задаче лучше использовать `std::sort`; если требуется показать собственную
реализацию с гарантированной асимптотикой, подходит heap sort.

## Документация

- HTML-документация: [docs/html/index.html](docs/html/index.html)
- Ссылка на GitHub: https://github.com/SirotaArt/CPP_Sort/tree/main/CPP_Sort_Lab_1
- Конфигурация для Doxygen
 
