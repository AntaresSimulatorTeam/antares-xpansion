#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <study_path>" >&2
    exit 1
fi

STUDY_PATH="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== Running plot_benders_duration ==="
python "$SCRIPT_DIR/plot_benders_duration.py" "$STUDY_PATH"

echo "=== Running load_added_constraints ==="
python "$SCRIPT_DIR/load_added_constraints.py" "$STUDY_PATH"

echo "=== Running plot_constraints_per_master ==="
python "$SCRIPT_DIR/plot_constraints_per_master.py" "$STUDY_PATH"

echo "=== All done ==="
