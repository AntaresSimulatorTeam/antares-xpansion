import json
import os
import shutil
import subprocess
from pathlib import Path

import pytest
import numpy as np


DATA_TEST = Path(__file__).parent.parent.parent.parent / "data_test"
RESULTS_FILE = "expected_results.json"


@pytest.fixture
def study_path(tmp_path, request):
    source_study = DATA_TEST / request.param
    test_study = tmp_path / request.param
    shutil.copytree(source_study, test_study)

    yield test_study


@pytest.mark.parametrize("study_path", [
    "simulator_full_gems_invest_13_1",
    "simulator_hybrid_invest_13_1",
], indirect=True)
def test_gems_end_to_end(install_dir, study_path, allow_run_as_root):
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

    output_dir = study_path / "output"
    expansion_dir = output_dir / "expansion"

    results_path = study_path / RESULTS_FILE
    with open(results_path, 'r') as f:
        expected_results = json.load(f)

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

    solution = output_data["solution"]

    actual_cost = solution.get("overall_cost")
    expected_cost = expected_results["overall_cost"]
    assert actual_cost is not None, "No overall_cost in solution"
    np.testing.assert_allclose(actual_cost, expected_cost, rtol=1e-6, atol=0,
                               err_msg=f"Overall cost mismatch: expected {expected_cost}, got {actual_cost}")

    assert "values" in solution, "No values in solution"
    expected_vars = expected_results["variables"]
    actual_values = solution["values"]

    expected_sorted = np.array([expected_vars[k] for k in sorted(expected_vars.keys())])
    actual_sorted = np.array([actual_values[k] for k in sorted(expected_vars.keys())])

    np.testing.assert_allclose(actual_sorted, expected_sorted, rtol=1e-6, atol=0,
                               err_msg="Variable values mismatch")

    print(f"Test passed! Solution status: {solution['problem_status']}")
    print(f"Overall cost: {solution.get('overall_cost')}")
