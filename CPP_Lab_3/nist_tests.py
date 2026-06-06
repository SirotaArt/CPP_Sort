import argparse
import csv
import math
import os

def erfc(x):
    return math.erfc(x)


def gammainc_upper(a, x):
    if x < 0 or a <= 0:
        return 1.0
    if x < a + 1:
        ap = a
        s = 1.0 / a
        delta = s
        for _ in range(1000):
            ap += 1
            delta *= x / ap
            s += delta
            if abs(delta) < abs(s) * 1e-12:
                break
        return 1.0 - s * math.exp(-x + a * math.log(x) - math.lgamma(a))
    b = x + 1.0 - a
    c = 1e300
    d = 1.0 / b
    h = d
    for i in range(1, 1000):
        an = -i * (i - a)
        b += 2.0
        d = an * d + b
        if abs(d) < 1e-300: d = 1e-300
        c = b + an / c
        if abs(c) < 1e-300: c = 1e-300
        d = 1.0 / d
        delta = d * c
        h *= delta
        if abs(delta - 1.0) < 1e-12: break
    return math.exp(-x + a * math.log(x) - math.lgamma(a)) * h

def bits_from_file(path, max_bits=None):
    bits = []
    with open(path, "rb") as f:
        data = f.read()
    for byte in data:
        for k in range(8):
            bits.append((byte >> (7 - k)) & 1)
            if max_bits is not None and len(bits) >= max_bits:
                return bits
    return bits


def t_monobit(bits):
    n = len(bits)
    s = sum(1 if b else -1 for b in bits)
    p = erfc(abs(s) / math.sqrt(2 * n))
    return p


def t_block_frequency(bits, M=128):
    n = len(bits)
    N = n // M
    if N == 0: return 0.0
    chi = 0.0
    for i in range(N):
        block = bits[i * M:(i + 1) * M]
        pi = sum(block) / M
        chi += (pi - 0.5) ** 2
    chi *= 4 * M
    return gammainc_upper(N / 2.0, chi / 2.0)


def t_runs(bits):
    n = len(bits)
    pi = sum(bits) / n
    if abs(pi - 0.5) >= 2 / math.sqrt(n):
        return 0.0
    Vn = 1 + sum(1 for i in range(1, n) if bits[i] != bits[i - 1])
    num = abs(Vn - 2 * n * pi * (1 - pi))
    den = 2 * math.sqrt(2 * n) * pi * (1 - pi)
    return erfc(num / den)


def t_longest_run_block(bits):
    n = len(bits)
    if n < 6272:
        return _longest_run_short(bits)
    M = 128
    K = 5
    V_buckets = [0, 0, 0, 0, 0, 0]
    pi_probs  = [0.1174, 0.2430, 0.2493, 0.1752, 0.1027, 0.1124]
    N = n // M
    for i in range(N):
        block = bits[i * M:(i + 1) * M]
        longest = cur = 0
        for b in block:
            cur = cur + 1 if b else 0
            if cur > longest: longest = cur
        # категории: <=4, 5, 6, 7, 8, >=9
        if   longest <= 4: V_buckets[0] += 1
        elif longest == 5: V_buckets[1] += 1
        elif longest == 6: V_buckets[2] += 1
        elif longest == 7: V_buckets[3] += 1
        elif longest == 8: V_buckets[4] += 1
        else:              V_buckets[5] += 1
    chi = sum((V_buckets[i] - N * pi_probs[i]) ** 2 / (N * pi_probs[i])
              for i in range(6))
    return gammainc_upper(K / 2.0, chi / 2.0)


def _longest_run_short(bits):
    M = 8; K = 3
    pi_probs = [0.2148, 0.3672, 0.2305, 0.1875]
    N = len(bits) // M
    if N == 0: return 0.0
    V = [0, 0, 0, 0]
    for i in range(N):
        block = bits[i * M:(i + 1) * M]
        longest = cur = 0
        for b in block:
            cur = cur + 1 if b else 0
            if cur > longest: longest = cur
        if   longest <= 1: V[0] += 1
        elif longest == 2: V[1] += 1
        elif longest == 3: V[2] += 1
        else:              V[3] += 1
    chi = sum((V[i] - N * pi_probs[i]) ** 2 / (N * pi_probs[i]) for i in range(4))
    return gammainc_upper(K / 2.0, chi / 2.0)


def t_cumulative_sums(bits, mode="forward"):
    n = len(bits)
    X = [1 if b else -1 for b in bits]
    if mode == "reverse":
        X.reverse()
    s = 0
    z = 0
    for x in X:
        s += x
        if abs(s) > z: z = abs(s)

    sqrtn = math.sqrt(n)
    s1 = sum(_phi((4 * k + 1) * z / sqrtn) - _phi((4 * k - 1) * z / sqrtn)
             for k in range(-int(((-n / z + 1) / 4)), int((n / z - 1) / 4) + 1))
    s2 = sum(_phi((4 * k + 3) * z / sqrtn) - _phi((4 * k + 1) * z / sqrtn)
             for k in range(-int(((-n / z - 3) / 4)), int((n / z - 1) / 4) + 1))
    return max(0.0, min(1.0, 1.0 - s1 + s2))


def _phi(x):
    return 0.5 * (1 + math.erf(x / math.sqrt(2)))


def run_suite(bits):
    return {
        "monobit":     t_monobit(bits),
        "block_freq":  t_block_frequency(bits),
        "runs":        t_runs(bits),
        "longest_run": t_longest_run_block(bits),
        "cumsum":      t_cumulative_sums(bits),
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("samples_dir")
    ap.add_argument("out_csv")
    ap.add_argument("--bits", type=int, default=1_000_000,
                    help="сколько бит читать из bits_*.bin")
    args = ap.parse_args()

    rows = []
    for fname in sorted(os.listdir(args.samples_dir)):
        if not fname.startswith("bits_") or not fname.endswith(".bin"):
            continue
        path = os.path.join(args.samples_dir, fname)
        gen  = fname[len("bits_"):-len(".bin")]
        bits = bits_from_file(path, max_bits=args.bits)
        print(f"  {gen}: {len(bits)} bits")
        res = run_suite(bits)
        row = {"generator": gen, **res,
               "passes_5": sum(1 for p in res.values() if p >= 0.01)}
        rows.append(row)
        for k, v in res.items():
            print(f"    {k:<12} p={v:.4f}  {'PASS' if v >= 0.01 else 'FAIL'}")

    os.makedirs(os.path.dirname(args.out_csv) or ".", exist_ok=True)
    with open(args.out_csv, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["generator", "monobit", "block_freq",
                                          "runs", "longest_run", "cumsum",
                                          "passes_5"])
        w.writeheader(); w.writerows(rows)
    print(f"written {args.out_csv}")


if __name__ == "__main__":
    main()
