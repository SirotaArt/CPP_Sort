import argparse
import csv
import os
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bench_csv")
    ap.add_argument("out_png")
    args = ap.parse_args()

    by_gen = defaultdict(list)
    with open(args.bench_csv, "r", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            by_gen[r["generator"]].append((int(r["size"]), float(r["time_sec"])))

    fig, ax = plt.subplots(figsize=(9, 6))
    markers = {"LCG_Xor": "o", "MidSquare_Improved": "s",
               "XorShift32_StarPlus": "^", "std_mt19937": "x"}
    for gen, points in by_gen.items():
        points.sort()
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        ax.plot(xs, ys, marker=markers.get(gen, "o"), label=gen,
                linewidth=1.4, markersize=5)
    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_xlabel("Количество сгенерированных чисел")
    ax.set_ylabel("Время генерации, секунд")
    ax.set_title("Скорость генерации: свои методы vs std::mt19937 (log-log)")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend()
    fig.tight_layout()
    os.makedirs(os.path.dirname(args.out_png) or ".", exist_ok=True)
    fig.savefig(args.out_png, dpi=150)
    print(f"saved {args.out_png}")


if __name__ == "__main__":
    main()
