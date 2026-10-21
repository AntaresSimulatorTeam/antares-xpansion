"""Plot stacked micro-iteration time per Benders iteration, one chart per sub."""

import re
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.patches import Patch

LOG_FILE = Path(__file__).parent / "micro_iterations_proc_0.log"


def parse_log(path: Path):
    """Return {sub_name: {benders_iter: {micro_iter: time_ms}}}."""
    data = defaultdict(lambda: defaultdict(dict))
    with open(path) as f:
        for line in f:
            m = re.match(
                r"(sub/sub_\d+\.mps)\s*;\s*(\d+)\s*;\s*(\d+)\s*;\s*\d+\s*;\s*(\d+)\s*ms",
                line.strip(),
            )
            if m:
                sub = m.group(1)
                benders = int(m.group(2))
                micro = int(m.group(3))
                time_ms = int(m.group(4))
                data[sub][benders][micro] = time_ms
    return data


def plot(data):
    subs = sorted(data.keys(), key=lambda s: int(re.search(r"\d+", s).group()))
    # Find max micro-iteration across all subs
    max_micro = max(mi for sub in data.values() for b in sub.values() for mi in b)
    colors = plt.colormaps["tab10"].resampled(max_micro)(range(max_micro))

    for sub in subs:
        sub_data = data[sub]
        benders_iters = sorted(sub_data.keys())
        x = range(len(benders_iters))

        fig, ax = plt.subplots(figsize=(10, 5))
        bottoms = [0.0] * len(benders_iters)

        for mi in range(1, max_micro + 1):
            heights = [sub_data[b].get(mi, 0) for b in benders_iters]
            ax.bar(x, heights, bottom=bottoms, color=colors[mi - 1], edgecolor="black",
                   linewidth=0.5)
            bottoms = [b + h for b, h in zip(bottoms, heights)]

        ax.set_xticks(x)
        ax.set_xticklabels([str(b) for b in benders_iters])
        ax.set_xlabel("Benders Iteration")
        ax.set_ylabel("Accumulated Micro-Iteration Time (ms)")
        ax.set_title(f"{sub} — Micro-Iteration Time per Benders Iteration")

        legend_handles = [Patch(facecolor=colors[i], edgecolor="black",
                                label=f"Micro-iter {i + 1}") for i in range(max_micro)]
        ax.legend(handles=legend_handles, title="Micro Iteration",
                  bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=8)

        plt.tight_layout()
        out = Path(__file__).parent / f"plot_{sub.replace('/', '_').replace('.mps', '')}.png"
        fig.savefig(out, dpi=150, bbox_inches="tight")
        plt.close(fig)
        print(f"Saved {out}")


if __name__ == "__main__":
    data = parse_log(LOG_FILE)
    plot(data)
