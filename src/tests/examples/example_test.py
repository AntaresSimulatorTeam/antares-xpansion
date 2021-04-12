from pathlib import Path
import sys
import shutil
import json

import numpy as np
import subprocess

import pytest

ALL_STUDIES_PATH = Path('../../../examples')


def get_first_json_filepath_output(output_dir):
    op = []
    for path in Path(output_dir).iterdir():
        for jsonpath in Path(path / "lp").rglob('out.json'):
            op.append(jsonpath)
    assert len(op) == 1
    return op[0]


def remove_outputs(study_path):
    output_path = study_path / 'output'
    for f in Path(output_path).iterdir():
        if f.is_dir():
            shutil.rmtree(f)


def launch_xpansion(install_dir, study_path, method):
    # Clean study output
    remove_outputs(study_path)

    install_dir_full = str(Path(install_dir).resolve())

    command = [sys.executable, "../../src_python/launch.py", "--installDir", install_dir_full, "--dataDir",
               str(study_path), "--method", method, "--step", "full", "-n", "2"]
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)
    output = process.communicate()

    # Check return value
    assert process.returncode == 0


def verify_solution(study_path, expected_values, expected_investment_solution):
    output_path = study_path / 'output'
    json_path = get_first_json_filepath_output(output_path)

    json_file = open(str(json_path), 'r')
    json_data = json.load(json_file)

    solution = json_data["solution"]
    investment_solution = solution["values"]

    json_file.close()

    RELATIVE_TOLERANCE = 1e-4
    ABSOLUTE_TOLERANCE = 1
    np.testing.assert_allclose(solution["gap"], expected_values["gap"], rtol=RELATIVE_TOLERANCE,
                                   atol=ABSOLUTE_TOLERANCE)
    np.testing.assert_allclose(solution["investment_cost"], expected_values["investment_cost"], rtol=RELATIVE_TOLERANCE,
                                   atol=ABSOLUTE_TOLERANCE)
    np.testing.assert_allclose(solution["operational_cost"], expected_values["operational_cost"],
                               rtol=RELATIVE_TOLERANCE,
                                   atol=ABSOLUTE_TOLERANCE)
    np.testing.assert_allclose(solution["overall_cost"], expected_values["overall_cost"], rtol=RELATIVE_TOLERANCE,
                                   atol=ABSOLUTE_TOLERANCE)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), "Investment " + investment + " not found in solution"

        np.testing.assert_allclose(expected_investment_solution[investment], investment_solution[investment], rtol=RELATIVE_TOLERANCE,
                                   atol=0)


## TESTS ##
@pytest.mark.medium
def test_001_sequential(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-01"

    expected_values = {"gap": -1.1444091796875e-05, "investment_cost": 224599999.99999237,
                       "operational_cost": 22513656189.121899, "overall_cost": 22738256189.121891}
    expected_investment_solution = {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02}

    launch_xpansion(installDir, study_path, "sequential")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.medium
def test_001_mpibenders(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-01"

    expected_values = {"gap": -1.1444091796875e-05, "investment_cost": 224599999.99999237,
                       "operational_cost": 22513656189.121899, "overall_cost": 22738256189.121891}
    expected_investment_solution = {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02}

    launch_xpansion(installDir, study_path, "mpibenders")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.long
def test_002_sequential(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-02"

    expected_values = {"gap": -1.430511474609375e-06, "investment_cost": 289923157.63737416,
                       "operational_cost": 1065480518.5118535, "overall_cost": 1355403676.1492281}
    expected_investment_solution = {"battery": 5.66e+02, "peak1": 6.0e+02, "peak2": 1.0e+03, "pv": 4.4267733994e+02,
                                    "semibase1": 6.0e+02}

    launch_xpansion(installDir, study_path, "sequential")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.long
def test_002_mpibenders(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-02"

    expected_values = {"gap": -1.430511474609375e-06, "investment_cost": 289923157.63737416,
                       "operational_cost": 1065480518.5118535, "overall_cost": 1355403676.1492281}
    expected_investment_solution = {"battery": 5.66e+02, "peak1": 6.0e+02, "peak2": 1.0e+03, "pv": 4.4267733994e+02,
                                    "semibase1": 6.0e+02}

    launch_xpansion(installDir, study_path, "mpibenders")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.medium
def test_003_sequential(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-03"

    expected_values = {"gap": 241262.43024921417, "investment_cost": 185999999.99999905,
                       "operational_cost": 5777590545.5126762, "overall_cost": 5963590545.5126753}
    expected_investment_solution = {"peak": 1599.9999999999998, "transmission_line": 0.0, "semibase": 1000.0000000000002}

    launch_xpansion(installDir, study_path, "sequential")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.medium
def test_003_mpibenders(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-03"

    expected_values = {"gap": 241262.43024921417, "investment_cost": 185999999.99999905,
                       "operational_cost": 5777590545.5126762, "overall_cost": 5963590545.5126753}
    expected_investment_solution = {"peak": 1599.9999999999998, "transmission_line": 0.0, "semibase": 1000.0000000000002}

    launch_xpansion(installDir, study_path, "mpibenders")
    verify_solution(study_path, expected_values, expected_investment_solution)
    remove_outputs(study_path)