import sys
import json
from pathlib import Path


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <study_path>", file=sys.stderr)
        sys.exit(1)
    study = sys.argv[1]

    out_path = Path(study) / "results_benders" / "out.json"
    if not out_path.is_file():
        raise FileNotFoundError(f"No results_benders/out.json found in {study}")

    with open(out_path) as f:
        data = json.load(f)

    iterations = data["iterations"]
    iter_keys = sorted(iterations.keys(), key=int)

    total_master = 0.0
    total_sub = 0.0

    for k in iter_keys:
        it = iterations[k]
        total_master += it["master_duration"]
        total_sub += it["subproblem_duration"]
        total = total_master + total_sub
        print(f"Iteration {k:>3}: master={total_master*1000:.0f} ms  sub={total_sub*1000:.0f} ms  total={total*1000:.0f} ms")

    total = total_master + total_sub
    print(f"\nTotal: master={total_master*1000:.0f} ms  sub={total_sub*1000:.0f} ms  total={total*1000:.0f} ms")
