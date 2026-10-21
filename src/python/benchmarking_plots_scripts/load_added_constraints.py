import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np

def load_added_constraints(study_path: str) -> dict[str, list[tuple[str, int]]]:
    base = Path(study_path) / "added_constraints"
    if not base.is_dir():
        raise FileNotFoundError(f"No added_constraints directory found in {study_path}")
    # Find last master
    master_dirs = sorted(base.iterdir(), key=lambda d: int(d.name.split("_")[1]))
    last_master = master_dirs[-1]

    result = {}
    for sub_dir in sorted(last_master.iterdir(), key=lambda d: int(d.name.split("_")[1])):
        sub_name = sub_dir.name
        # Find second-to-last micro_iter (last is always empty)
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


def map_constraints_to_families(
    added: dict[str, list[tuple[str, int]]],
    dictionary: dict[str, list[str]],
) -> dict[str, list[tuple[str, int]]]:
    result = {}
    for sub, pairs in added.items():
        families = []
        for name, value in pairs:
            if name in dictionary:
                families.append((name, value))
            else:
                print(f"Warning: constraint '{name}' in {sub} not found in dictionary, ignoring")
        result[sub] = families
    return result


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <study_path>", file=sys.stderr)
        sys.exit(1)
    study = sys.argv[1]

    data = load_added_constraints(study)
    for sub, pairs in data.items():
        print(f"{sub}: {pairs}")

    cdict = load_constraints_dictionary(study)

    families_per_sub = map_constraints_to_families(data, cdict)

    # Count real number of constraints (sum of family sizes) grouped by value 0 or 1
    subs = sorted(families_per_sub.keys(), key=lambda s: int(s.split("_")[1]))
    counts_0 = []
    counts_1 = []
    for sub in subs:
        c0, c1 = 0, 0
        for name, value in families_per_sub[sub]:
            n_constraints = len(cdict[name])
            if value == 1:
                c1 += n_constraints
            else:
                c0 += n_constraints
        counts_0.append(c0)
        counts_1.append(c1)

    x = np.arange(len(subs))
    width = 0.35
    fig, ax = plt.subplots()
    ax.bar(x - width / 2, counts_0, width, label="value=0")
    ax.bar(x + width / 2, counts_1, width, label="value=1 (is_N)")
    ax.set_xlabel("Sub")
    ax.set_ylabel("Number of constraints")
    ax.set_title("Added constraints per sub")
    ax.set_xticks(x)
    ax.set_xticklabels(subs, rotation=45, ha="right")
    ax.legend()
    fig.tight_layout()
    plt.savefig(Path(study) / "added_constraints_histogram.png", dpi=150)
    plt.show()
