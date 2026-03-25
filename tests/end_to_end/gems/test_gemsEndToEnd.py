import json
import os
import shutil
import subprocess
from pathlib import Path

import pytest


DATA_TEST = Path("../../../data_test")
STUDY_NAME = "simulator_hybrid_invest_13_1"


@pytest.fixture
def study_path(tmp_path):
    source_study = DATA_TEST / STUDY_NAME
    test_study = tmp_path / STUDY_NAME
    shutil.copytree(source_study, test_study)

    expansion_dir = test_study / "expansion"
    if expansion_dir.exists():
        shutil.rmtree(expansion_dir)

    yield test_study


def test_gems_workflow(install_dir, study_path, allow_run_as_root):
    launch_py = Path(__file__).parent.parent.parent.parent / "src" / "python" / "launch.py"

    cmd = [
        "python3", str(launch_py),
        "-i", str(study_path),
        "--step", "gems",
        "--installDir", str(install_dir)
    ]

    if allow_run_as_root:
        cmd.append("--allow_run_as_root")

    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True
    )

    print(f"Return code: {result.returncode}")
    print(f"STDOUT:\n{result.stdout}")
    if result.stderr:
        print(f"STDERR:\n{result.stderr}")

    assert result.returncode == 0, f"Gems workflow failed with return code {result.returncode}"

    expansion_dir = study_path / "expansion"
    output_dir = study_path / "output"

    if not expansion_dir.exists() and output_dir.exists():
        for sim_dir in sorted(output_dir.iterdir(), key=lambda p: p.stat().st_mtime, reverse=True):
            candidate = sim_dir / "expansion"
            if candidate.exists():
                expansion_dir = candidate
                break

    assert expansion_dir.exists(), "expansion directory was not created"

    out_json = expansion_dir / "out.json"
    assert out_json.exists(), "expansion/out.json was not created"

    with open(out_json, 'r') as f:
        output_data = json.load(f)

    assert "solution" in output_data, "No solution in out.json"
    assert "problem_status" in output_data["solution"], "No problem_status in solution"
    assert output_data["solution"]["problem_status"] == "OPTIMAL", \
        f"Expected OPTIMAL status, got {output_data['solution']['problem_status']}"

    print(f"Test passed! Solution status: {output_data['solution']['problem_status']}")
    if "overall_cost" in output_data["solution"]:
        print(f"Overall cost: {output_data['solution']['overall_cost']}")