from argparse import ArgumentParser, Namespace
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def parse_args() -> Namespace:
    parser = ArgumentParser()
    parser.add_argument("-d", "--result_dir", type=Path, default=Path("results"), help="Directory path to the result")
    return parser.parse_args()


def read_result(dir_path: Path) -> np.ndarray:
    if not dir_path.is_dir():
        raise NotADirectoryError(f"{dir_path} is not a directory")
    with open(dir_path / "counts.bin", "rb") as f:
        count_data = f.read()
    counts = np.array([*count_data], dtype=np.uint8)
    return counts


def main() -> None:
    args = parse_args()
    counts = read_result(args.result_dir)

    print(counts[0])

    plt.figure(dpi=200)
    plt.hist(counts, bins=np.arange(0, 76))
    plt.savefig(args.result_dir / "histogram.png")

    count_dict = {}
    for i in range(76):
        count_dict[i] = (counts == i).sum()
    with open(args.result_dir / "count.txt", "w") as f:
        for i in range(76):
            f.write(f"{i:>2d}: {count_dict[i]}\n")


if __name__ == "__main__":
    main()
