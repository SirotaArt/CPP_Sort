import argparse
import csv
import math
import os
import statistics


CHI2_CRIT_005 = {  # верхние 5%-точки χ² (k степеней свободы)
     9: 16.919, 19: 30.144, 29: 42.557, 39: 54.572, 49: 66.339,
     99: 123.225, 199: 233.994,
}


def chi_square_uniform(values, low, high, bins):
    n = len(values)
    expected = n / bins
    counts = [0] * bins
    width = (high - low) / bins
    for v in values:
        idx = min(int((v - low) / width), bins - 1)
        counts[idx] += 1
    chi = sum((c - expected) ** 2 / expected for c in counts)
    df = bins - 1
    return chi, df, counts


def analyze_file(path, bins=20):
    with open(path, "r", encoding="utf-8") as f:
        values = [int(line) for line in f if line.strip()]
    mean = statistics.fmean(values)
    sd   = statistics.pstdev(values)
    cv   = (sd / mean) * 100 if mean else 0.0
    chi, df, _ = chi_square_uniform(values, 0, 5000, bins)
    crit = CHI2_CRIT_005.get(df, math.nan)
    passed = chi < crit
    return {
        "file":  os.path.basename(path),
        "n":     len(values),
        "mean":  round(mean, 4),
        "stdev": round(sd, 4),
        "cv_%":  round(cv, 4),
        "chi2":  round(chi, 4),
        "df":    df,
        "chi2_crit_5%": crit,
        "uniform_pass_5%": passed,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("samples_dir", help="results/samples")
    ap.add_argument("out_csv",     help="results/stats.csv")
    ap.add_argument("--bins", type=int, default=20)
    args = ap.parse_args()

    rows = []
    for gen in sorted(os.listdir(args.samples_dir)):
        d = os.path.join(args.samples_dir, gen)
        if not os.path.isdir(d): 
            continue
        for fname in sorted(os.listdir(d)):
            if not fname.startswith("sample_"):
                continue
            r = analyze_file(os.path.join(d, fname), bins=args.bins)
            r["generator"] = gen
            rows.append(r)
            print(f"  {gen}/{fname}: mean={r['mean']:.1f}  cv={r['cv_%']:.2f}%  "
                  f"chi2={r['chi2']:.2f}  uniform={'yes' if r['uniform_pass_5%'] else 'no'}")

    os.makedirs(os.path.dirname(args.out_csv) or ".", exist_ok=True)
    with open(args.out_csv, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=[
            "generator", "file", "n", "mean", "stdev", "cv_%",
            "chi2", "df", "chi2_crit_5%", "uniform_pass_5%"])
        w.writeheader(); w.writerows(rows)
    print(f"written {args.out_csv}  ({len(rows)} samples)")

    summary_path = os.path.splitext(args.out_csv)[0] + "_summary.csv"
    by_gen = {}
    for r in rows:
        by_gen.setdefault(r["generator"], []).append(r)
    with open(summary_path, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["generator", "samples", "mean_avg", "cv_avg",
                    "chi2_avg", "uniform_pass_count"])
        for g, arr in by_gen.items():
            w.writerow([g, len(arr),
                        round(sum(x["mean"] for x in arr) / len(arr), 2),
                        round(sum(x["cv_%"] for x in arr) / len(arr), 3),
                        round(sum(x["chi2"] for x in arr) / len(arr), 3),
                        sum(1 for x in arr if x["uniform_pass_5%"])])
    print(f"written {summary_path}")


if __name__ == "__main__":
    main()
