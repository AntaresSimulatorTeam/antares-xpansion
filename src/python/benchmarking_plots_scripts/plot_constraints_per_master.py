import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np


def load_constraints_dictionary(study_path: str) -> dict[str, list[str]]:
    csv_path = Path(study_path) / "plugin_inputs" / "constraints_dictionary.csv"
    if not csv_path.is_file():
        raise FileNotFoundError(f"No constraints_dictionary.csv found in {study_path}/plugin_inputs")
    result = {}
    for line in csv_path.read_text().splitlines():
        line = line.strip()
        if not line:
            continue
        parts = line.split(",")
        result[parts[0]] = parts[1:]
    return result


def load_added_for_master(master_dir: Path) -> dict[str, list[tuple[str, int]]]:
    result = {}
    for sub_dir in sorted(master_dir.iterdir(), key=lambda d: int(d.name.split("_")[1])):
        sub_name = sub_dir.name
        micro_iters = sorted(sub_dir.glob("micro_iter_*.txt"), key=lambda f: int(f.stem.split("_")[-1]))
        if len(micro_iters) < 2:
            result[sub_name] = []
            continue
        target = micro_iters[-2]
        pairs = []
        for line in target.read_text().splitlines():
            line = line.strip()
            if not line:
                continue
            name, value = line.rsplit(" ", 1)
            pairs.append((name, int(value)))
        result[sub_name] = pairs
    return result


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <study_path>", file=sys.stderr)
        sys.exit(1)
    study = sys.argv[1]

    cdict = load_constraints_dictionary(study)

    base = Path(study) / "added_constraints"
    if not base.is_dir():
        raise FileNotFoundError(f"No added_constraints directory found in {study}")

    master_dirs = sorted(base.iterdir(), key=lambda d: int(d.name.split("_")[1]))

    output_dir = Path(study) / "plots_per_master"
    output_dir.mkdir(exist_ok=True)

    for master_dir in master_dirs:
        master_name = master_dir.name
        data = load_added_for_master(master_dir)

        subs = sorted(data.keys(), key=lambda s: int(s.split("_")[1]))
        counts_0 = []
        counts_1 = []
        for sub in subs:
            c0, c1 = 0, 0
            for name, value in data[sub]:
                if name in cdict:
                    n_constraints = len(cdict[name])
                    if value == 1:
                        c1 += n_constraints
                    else:
                        c0 += n_constraints
            counts_0.append(c0)
            counts_1.append(c1)

        x = np.arange(len(subs))
        fig, ax = plt.subplots(figsize=(10, 5))
        ax.bar(x, counts_0, label="value=0", color="steelblue")
        ax.bar(x, counts_1, bottom=counts_0, label="value=1 (is_N)", color="coral")
        ax.set_xlabel("Sub")
        ax.set_ylabel("Number of constraints")
        ax.set_title(f"Added constraints — {master_name}")
        ax.set_xticks(x)
        ax.set_xticklabels(subs, rotation=45, ha="right")
        ax.legend()
        fig.tight_layout()
        plt.savefig(output_dir / f"{master_name}.png", dpi=150)
        plt.close(fig)
        print(f"Saved {master_name}.png")

    print(f"All plots saved to {output_dir}")
