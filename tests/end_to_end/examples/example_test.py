from pathlib import Path
import sys
import shutil
import json

import numpy as np
import subprocess

import pytest

from candidates_reader import CandidatesReader

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

    command = [sys.executable, "../../../src/python/launch.py", "--installDir", install_dir_full, "--dataDir",
               str(study_path), "--method", method, "--step", "full", "-n", "2"]
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)
    output = process.communicate()

    # Check return value
    assert process.returncode == 0


def verify_solution(study_path, expected_values, expected_investment_solution):
    output_path = study_path / 'output'
    json_path = get_first_json_filepath_output(output_path)

    with open(str(json_path), 'r') as json_file:
        json_data = json.load(json_file)

    solution = json_data["solution"]
    investment_solution = solution["values"]

    json_file.close()

    RELATIVE_TOLERANCE = 1e-4
    ABSOLUTE_TOLERANCE = 0.1
    
    np.testing.assert_allclose(solution["investment_cost"], expected_values["investment_cost"], rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(solution["operational_cost"], expected_values["operational_cost"], rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(solution["overall_cost"], expected_values["overall_cost"], rtol=RELATIVE_TOLERANCE)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), "Investment " + investment + " not found in solution"
        np.testing.assert_allclose(expected_investment_solution[investment], investment_solution[investment], rtol=RELATIVE_TOLERANCE)


def verify_study_update(study_path, expected_investment_solution):
    candidate_reader = CandidatesReader(study_path / "user" / "expansion" / "candidates.ini")
    for candidate in expected_investment_solution.keys():
        investment = expected_investment_solution[candidate]
        already_installed_capacity = candidate_reader.get_candidate_already_install_capacity(candidate)

        link_profile_array = candidate_reader.get_candidate_link_profile_array(study_path,candidate)
        already_installed_link_profile_array = candidate_reader.get_candidate_already_installed_link_profile_array(study_path,candidate)
        assert link_profile_array.shape == already_installed_link_profile_array.shape

        expected_link_capacity = already_installed_capacity * already_installed_link_profile_array + investment * link_profile_array

        study_link = candidate_reader.get_candidate_antares_link_file(study_path, candidate)
        study_link_array = np.loadtxt(study_link, delimiter="\t")
        link_capacity = study_link_array[:, [0,1]]

        np.testing.assert_allclose(link_capacity,expected_link_capacity, rtol=1e-4)


def test_full_study(installDir, study_path, expected_values, expected_investment_solution, solver : str):
    launch_xpansion(installDir, study_path, solver)
    verify_solution(study_path, expected_values, expected_investment_solution)
    verify_study_update(study_path, expected_investment_solution)
    remove_outputs(study_path)

## TESTS ##
@pytest.mark.parametrize(
    "study_path, expected_values, expected_investment_solution",
    [
        (ALL_STUDIES_PATH / "xpansion-test-02",
         {"gap": -1.430511474609375e-06, "investment_cost": 289923157.63737416,
          "operational_cost": 1065480518.5118535, "overall_cost": 1355403676.1492281},
         {"battery": 5.66e+02, "peak1": 6.0e+02, "peak2": 1.0e+03, "pv": 4.4267733994e+02,
          "semibase1": 6.0e+02}
         ),
        (ALL_STUDIES_PATH / "xpansion-test-02-new",
         {"gap": 1050.4630239009857, "investment_cost": 255970392.38226271, "operational_cost": 1067647043.6995589,
          "overall_cost": 1323617436.0818214},
         {"battery": 503.64036279603744, "peak1": 800.0, "peak2": 1000.0, "pv": 446.78196988044607,
          "semibase1": 0.0,"semibase2": 200.0}
         )
    ],
)
@pytest.mark.long
def test_full_study_long(installDir, study_path, expected_values, expected_investment_solution):
    test_full_study(installDir, study_path, expected_values, expected_investment_solution, "sequential")
    test_full_study(installDir, study_path, expected_values, expected_investment_solution, "mpibenders")

@pytest.mark.parametrize(
        "study_path, expected_values, expected_investment_solution",
        [
            (ALL_STUDIES_PATH / "xpansion-test-01",
             {"gap": -1.1444091796875e-05, "investment_cost": 224599999.99999237,
              "operational_cost": 22513656189.121899, "overall_cost": 22738256189.121891},
             {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02}
             ),
            (ALL_STUDIES_PATH / "xpansion-test-03",
             {"gap": 241262.43024921417, "investment_cost": 185999999.99999905, "operational_cost": 5777590545.5126762, "overall_cost": 5963590545.5126753},
             {"peak": 1599.9999999999998, "transmission_line": 0.0, "semibase": 1000.0000000000002}
             ),
            (ALL_STUDIES_PATH / "xpansion-test-04-mps-rounding",
             {"gap": -0.000560760498046875, "investment_cost": 115399999.99998856, "operational_cost": 21942457893.943958, "overall_cost": 22057857893.943947},
             {"battery": 1000.0000000000124, "peak": 0.0, "pv": 1000.0, "semibase": 0.0, "transmission_line": 0.0}
             )
        ],
    )
@pytest.mark.medium
def test_full_study_medium(installDir, study_path, expected_values,expected_investment_solution):
    test_full_study(installDir, study_path, expected_values, expected_investment_solution, "sequential")
    test_full_study(installDir, study_path, expected_values, expected_investment_solution, "mpibenders")
