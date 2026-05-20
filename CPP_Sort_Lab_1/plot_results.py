import argparse
import csv
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_bench(path: str):
    sizes, sel, bub, heap, std = [], [], [], [], []
    with open(path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            sizes.append(int(row["size"]))
            sel.append(float(row["selection"]))
            bub.append(float(row["bubble"]))
            heap.append(float(row["heap"]))
            std.append(float(row["std_sort"]))

    idx = sorted(range(len(sizes)), key=lambda i: sizes[i])
    pick = lambda xs: [xs[i] for i in idx]
    return pick(sizes), pick(sel), pick(bub), pick(heap), pick(std)


def _filter(xs, ys):

    fx, fy = [], []
    for x, y in zip(xs, ys):
        if y >= 0:
            fx.append(x); fy.append(y)
    return fx, fy


def make_plot(bench_csv: str, out_png: str, log: bool = False) -> None:
    sizes, sel, bub, heap, std = read_bench(bench_csv)
    fig, ax = plt.subplots(figsize=(9, 6))

    for label, ys, marker in [
        ("Selection Sort", sel,  "o"),
        ("Bubble Sort",    bub,  "s"),
        ("Heap Sort",      heap, "^"),
        ("std::sort",      std,  "x"),
    ]:
        fx, fy = _filter(sizes, ys)
        if fx:
            ax.plot(fx, fy, marker=marker, label=label, linewidth=1.4, markersize=5)

    ax.set_xlabel("Размер массива, элементов")
    ax.set_ylabel("Время сортировки, секунд")
    ax.set_title("Время сортировок lottery_ticket в зависимости от N")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    if log:
        ax.set_xscale("log"); ax.set_yscale("log")
        ax.set_title(ax.get_title() + "  (log-log)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_png, dpi=150)
    print(f"saved {out_png}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bench_csv")
    ap.add_argument("out_png")
    ap.add_argument("--log", action="store_true", help="log-log оси")
    args = ap.parse_args()
    os.makedirs(os.path.dirname(args.out_png) or ".", exist_ok=True)
    make_plot(args.bench_csv, args.out_png, log=args.log)


if __name__ == "__main__":
    main()
