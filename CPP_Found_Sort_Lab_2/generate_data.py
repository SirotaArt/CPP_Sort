import argparse
import csv
import os
import random
from datetime import date, timedelta

DEFAULT_SIZES = [100, 250, 500, 1000, 2000, 4000, 6000, 8000, 10000, 15000, 20000, 30000, 50000, 70000, 100000]

MAIN_FILE_SIZE = 100000

HEADER = ["ticketNumber", "nominalValue", "lotteryDate", "winAmount"]


def synthesize(count: int, start_index: int = 1, seed: int = 42) -> list[tuple[str, int, str, int]]:
    rng = random.Random(seed)
    first_day = date(2022, 1, 1)
    rows = []

    for i in range(count):
        ticket_number = f"{start_index + i:08d}"
        nominal_value = rng.choice([50, 100, 150, 200, 500])
        lottery_date = first_day + timedelta(days=rng.randint(0, 365 * 4))
        win_amount = 0
        
        if rng.random() < 0.10:
            win_amount = rng.choice([nominal_value, nominal_value * 5, nominal_value * 10, nominal_value * 100, 100000])

        rows.append((ticket_number, nominal_value, lottery_date.isoformat(), win_amount))

    return rows


def write_csv(path: str, rows: list[tuple[str, int, str, int]]) -> str:
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(HEADER)
        writer.writerows(rows)
    return path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--outdir", default="data")
    parser.add_argument("--sizes", default=",".join(map(str, DEFAULT_SIZES)))
    args = parser.parse_args()

    sizes = sorted({int(x) for x in args.sizes.split(",") if x.strip()})
    required_rows = max(max(sizes), MAIN_FILE_SIZE)

    os.makedirs(args.outdir, exist_ok=True)
    rows = synthesize(required_rows)
    print(f"generated {len(rows)} base records")

    for size in sizes:
        path = write_csv(os.path.join(args.outdir, f"lottery_{size}.csv"), rows[:size])
        print(f"  wrote {path}")

    main_path = os.path.join(args.outdir, f"lottery_{MAIN_FILE_SIZE}.csv")
    if MAIN_FILE_SIZE not in sizes:
        write_csv(main_path, rows[:MAIN_FILE_SIZE])
        print(f"  wrote {main_path}")


if __name__ == "__main__":
    main()
