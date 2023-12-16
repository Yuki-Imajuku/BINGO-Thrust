from argparse import ArgumentParser, Namespace
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def parse_args() -> Namespace:
    parser = ArgumentParser()
    parser.add_argument("-d", "--result_dir", type=Path, default=Path("results"), help="Directory path to the result")
    return parser.parse_args()


def read_result(dir_path: Path) -> tuple[np.ndarray, np.ndarray]:
    if not dir_path.is_dir():
        raise NotADirectoryError(f"{dir_path} is not a directory")
    with open(dir_path / "counts.bin", "rb") as f:
        count_data = f.read()
    counts = np.array([*count_data], dtype=np.uint8)
    with open(dir_path / "boards.bin", "rb") as f:
        board_data = f.read()
    boards = np.array([*board_data], dtype=np.uint8)
    boards = np.repeat(boards, 2, axis=0).reshape(-1, 13, 2).transpose(0, 2, 1)
    boards[:, 0] >>= 4  # upper 4 bits
    boards[:, 1] &= 0x0F  # lower 4 bits
    boards = boards.transpose(0, 2, 1).reshape(-1, 26)[:, 24::-1].reshape(-1, 5, 5)  # Discard the last element and flip and reshape
    boards += np.arange(0, 75, 15, dtype=np.uint8).reshape(1, -1, 1)
    return counts, boards


def main() -> None:
    args = parse_args()
    counts, boards = read_result(args.result_dir)

    print(counts[0])
    print(boards[0])

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
