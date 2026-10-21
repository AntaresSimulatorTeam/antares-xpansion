import os
import re

CONSTRAINTS_DIR = os.path.join(os.path.dirname(__file__), "added_constraints")


def read_last_constraints():
    """Read constraints from the last master iteration, then last micro iteration per sub.
    Returns a dict: sub name -> list of (constraint_family, is_N) tuples."""

    pattern = re.compile(r"micro_iter_(\d+)_master_(\d+)_sub_(\d+)\.txt")

    # Find global max master iteration
    max_master = 0
    for fname in os.listdir(CONSTRAINTS_DIR):
        m = pattern.match(fname)
        if m:
            max_master = max(max_master, int(m.group(2)))

    # Among files with max master, find max micro iteration per sub
    max_micro_per_sub = {}
    for fname in os.listdir(CONSTRAINTS_DIR):
        m = pattern.match(fname)
        if m:
            micro, master, sub = int(m.group(1)), int(m.group(2)), int(m.group(3))
            if master == max_master:
                sub_key = f"sub_{sub}"
                if sub_key not in max_micro_per_sub or micro > max_micro_per_sub[sub_key]:
                    max_micro_per_sub[sub_key] = micro

    # Read the target files
    result = {}
    for sub_key, micro in max_micro_per_sub.items():
        fname = f"micro_iter_{micro}_master_{max_master}_{sub_key}.txt"
        fpath = os.path.join(CONSTRAINTS_DIR, fname)

        constraints = []
        with open(fpath) as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.rsplit(" ", 1)
                constraints.append((parts[0], int(parts[1])))

        result[sub_key] = constraints

    return result


if __name__ == "__main__":
    data = read_last_constraints()
    for sub in sorted(data, key=lambda s: int(s.split("_")[1])):
        print(f"{sub}: {len(data[sub])} constraints")
        for family, is_n in data[sub][:5]:
            print(f"  {family} {is_n}")
        if len(data[sub]) > 5:
            print(f"  ... ({len(data[sub]) - 5} more)")
