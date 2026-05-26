import argparse
import csv
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read(path):
    rows = []
    with open(path, "r", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            rows.append({k: float(v) for k, v in r.items()})
    rows.sort(key=lambda r: r["size"])
    return rows


def plot_search(rows, out_png, log=False):
    xs = [r["size"] for r in rows]
    cols = [
        ("Linear",  "search_linear_avg", "o"),
        ("BST",     "search_bst_avg",    "s"),
        ("RB-tree", "search_rb_avg",     "^"),
        ("Hash",    "search_hash_avg",   "x"),
        ("multimap", "search_multimap_avg", "D"),
    ]
    fig, ax = plt.subplots(figsize=(9, 6))
    for label, key, marker in cols:
        if key not in rows[0]:
            continue
        ax.plot(xs, [r[key] for r in rows], marker=marker, label=label,
                linewidth=1.4, markersize=5)
    ax.set_xlabel("Размер массива N")
    ax.set_ylabel("Среднее время одного поиска, секунд")
    ax.set_title("Поиск всех вхождений по ключу lotteryDate")
    if log:
        ax.set_xscale("log"); ax.set_yscale("log")
        ax.set_title(ax.get_title() + "  (log-log)")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_png, dpi=150)
    print(f"saved {out_png}")


def plot_build(rows, out_png):
    xs = [r["size"] for r in rows]
    cols = [
        ("BST build",     "build_bst",  "s"),
        ("RB-tree build", "build_rb",   "^"),
        ("Hash build",    "build_hash", "x"),
        ("multimap build", "build_multimap", "D"),
    ]
    fig, ax = plt.subplots(figsize=(9, 6))
    for label, key, marker in cols:
        if key not in rows[0]:
            continue
        ax.plot(xs, [r[key] for r in rows], marker=marker, label=label,
                linewidth=1.4, markersize=5)
    ax.set_xlabel("Размер массива N")
    ax.set_ylabel("Время построения индекса, секунд")
    ax.set_title("Построение индексов поиска")
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_png, dpi=150)
    print(f"saved {out_png}")


def plot_hash_collisions(rows, out_png):
    if "hash_collisions" not in rows[0]:
        return
    xs = [r["size"] for r in rows]
    fig, ax1 = plt.subplots(figsize=(9, 6))
    ax1.plot(xs, [r["hash_collisions"] for r in rows],
             marker="o", linewidth=1.4, label="Hash collisions")
    ax1.set_xlabel("Размер массива N")
    ax1.set_ylabel("Число коллизий разных ключей")
    ax1.grid(True, which="both", linestyle="--", alpha=0.5)
    ax1.set_xscale("log")

    ax2 = ax1.twinx()
    ax2.plot(xs, [r["hash_max_chain"] for r in rows],
             color="tab:red", marker="s", linewidth=1.4, label="Max chain")
    ax2.set_ylabel("Максимальная длина цепочки")

    lines = ax1.get_lines() + ax2.get_lines()
    ax1.legend(lines, [line.get_label() for line in lines], loc="best")
    ax1.set_title("Коллизии хэш-функции FNV-1a по ключу lotteryDate")
    fig.tight_layout()
    fig.savefig(out_png, dpi=150)
    print(f"saved {out_png}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bench_csv")
    ap.add_argument("out_dir")
    args = ap.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    rows = read(args.bench_csv)
    plot_search(rows, os.path.join(args.out_dir, "search_linear.png"), log=False)
    plot_search(rows, os.path.join(args.out_dir, "search_loglog.png"), log=True)
    plot_build (rows, os.path.join(args.out_dir, "build.png"))
    plot_hash_collisions(rows, os.path.join(args.out_dir, "hash_collisions.png"))


if __name__ == "__main__":
    main()
