import os
from pathlib import Path
import sys
import shutil
import json

import numpy as np
import subprocess

import pytest

from candidates_reader import CandidatesReader

ALL_STUDIES_PATH = Path('../../../data_test/examples')


def get_first_json_filepath_output(output_dir):
    op = []
    for path in Path(output_dir).iterdir():
        for jsonpath in Path(path / "lp").rglob('out.json'):
            op.append(jsonpath)
    assert len(op) == 1
    return op[0]


def remove_outputs(study_path):
    output_path = study_path / 'output'
    if os.path.isdir(output_path):
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
    np.testing.assert_allclose(solution["operational_cost"], expected_values["operational_cost"],
                               rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(solution["overall_cost"], expected_values["overall_cost"], rtol=RELATIVE_TOLERANCE)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), "Investment " + investment + " not found in solution"
        np.testing.assert_allclose(expected_investment_solution[investment], investment_solution[investment],
                                   rtol=RELATIVE_TOLERANCE)


def verify_study_update(study_path, expected_investment_solution):
    candidate_reader = CandidatesReader(study_path / "user" / "expansion" / "candidates.ini")
    for link in candidate_reader.get_link_list():
        candidate_name_list = candidate_reader.get_link_candidate(link)
        already_installed_capacity = candidate_reader.get_candidate_already_install_capacity(candidate_name_list[0])
        already_installed_link_profile_array = candidate_reader.get_candidate_already_installed_link_profile_array(
            study_path, candidate_name_list[0])
        expected_link_capacity = already_installed_capacity * already_installed_link_profile_array

        for candidate in candidate_name_list:
            investment = expected_investment_solution[candidate]
            link_profile_array = candidate_reader.get_candidate_link_profile_array(study_path, candidate)
            assert link_profile_array.shape == already_installed_link_profile_array.shape
            expected_link_capacity += investment * link_profile_array

        study_link = candidate_reader.get_link_antares_link_file(study_path, link)
        study_link_array = np.loadtxt(study_link, delimiter="\t")
        link_capacity = study_link_array[:, [0, 1]]

        np.testing.assert_allclose(link_capacity, expected_link_capacity, rtol=1e-4)


def verify_full_study(install_dir, study_path, expected_values, expected_investment_solution, solver: str):
    launch_xpansion(install_dir, study_path, solver)
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
          "semibase1": 0.0, "semibase2": 200.0}
         ),
        (ALL_STUDIES_PATH / "xpansion-test-05-area-uppercase",
         {"gap": 168.89355158805847, "investment_cost": 433632217.73959851, "operational_cost": 1348247642.9924254,
          "overall_cost": 1781879860.732024},
         {"elec_grid": 526.32217750589962, "h2_grid": 600.00000026293719, "p2g_marg_area1": 1124.9999998864932,
          "p2g_marg_area2": 2000.0}
         )
    ],
)
@pytest.mark.long
def test_full_study_long(install_dir, study_path, expected_values, expected_investment_solution):
    verify_full_study(install_dir, study_path, expected_values, expected_investment_solution, "sequential")
    verify_full_study(install_dir, study_path, expected_values, expected_investment_solution, "mpibenders")


medium_parameters_names = "study_path, expected_values, expected_investment_solution"
medium_paramters_values = [
    (ALL_STUDIES_PATH / "xpansion-test-01",
     {"gap": -1.1444091796875e-05, "investment_cost": 224599999.99999237,
      "operational_cost": 22513656189.121899, "overall_cost": 22738256189.121891},
     {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02, "transmission_line": 0.0}
     ),
    (ALL_STUDIES_PATH / "xpansion-test-03",
     {"gap": -1.1920928955078125e-06, "investment_cost": 201999999.99999976, "operational_cost": 1161894495.7433052,
      "overall_cost": 1363894495.743305},
     {"peak": 2500.0, "transmission_line": 1600.0, "semibase": 400.0}
     ),
    (ALL_STUDIES_PATH / "xpansion-test-04-mps-rounding",
     {"gap": -0.000560760498046875, "investment_cost": 115399999.99998856, "operational_cost": 21942457893.943958,
      "overall_cost": 22057857893.943947},
     {"battery": 1000.0000000000124, "peak": 0.0, "pv": 1000.0, "semibase": 0.0, "transmission_line": 0.0}
     ),
    (ALL_STUDIES_PATH / "xpansion-test-01-weights",
     {"gap": 3.814697265625e-06, "investment_cost": 230600000.00002289, "operational_cost": 24001577891.450424,
      "overall_cost": 24232177891.450447},
     {"battery": 1000.0, "peak": 1500.0, "pv": 1000.0, "semibase": 200.0, "transmission_line": 0.0}
     ),
    (ALL_STUDIES_PATH / "xpansion-test-one-link-two-candidates",
     {"gap": -1.9073486328125e-06, "investment_cost": 15999999.999994278, "operational_cost": 11800333780.786646,
      "overall_cost": 11816333780.78664},
     {"transmission_line": 800.0, "transmission_line_2": 800.0}
     ),
    (ALL_STUDIES_PATH / "xpansion-test-01-hurdles-cost",
     {"gap": -7.62939453125e-06, "investment_cost": 224599999.99996948,
      "operational_cost": 22516674015.060184, "overall_cost": 22741274015.060154},
     {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02, "transmission_line": 0.0}
     ),
    (ALL_STUDIES_PATH / "additionnal-constraints",
     {"gap": 3.0517578125e-05, "investment_cost": 224600000.00003052,
      "operational_cost": 22513655727.899261, "overall_cost": 22738255727.899292},
     {"battery": 1.0e+03, "peak": 1.4e+03, "pv": 1.0e+03, "semibase": 2.0e+02, "transmission_line": 0.0}
     ),
    (ALL_STUDIES_PATH / "additionnal-constraints-binary",
     {"gap": -7.62939453125e-06, "investment_cost": 220499999.99999809,
      "operational_cost": 13501512886.702877, "overall_cost": 13722012886.702875},
     {"battery": 885, "peak": 1800, "pv": 1.0e+03, "semibase": 0.0, "transmission_line": 400.0}
     ),
    (ALL_STUDIES_PATH / "hurdles-cost-profile-value-over-one",
     {"gap": 239316.41654706001, "investment_cost": 231999999.99999976,
      "operational_cost": 1089603112.1225781, "overall_cost": 1321603112.1225779},
     {"peak": 1300.0, "semibase": 1600.0, "transmission_line": 1000.0}
     ),
    (ALL_STUDIES_PATH / "link-profile-with-empty-week",
     {"gap": 0.00983428955078125, "investment_cost": 27585000000.000111,
      "operational_cost": 10629896636.068903, "overall_cost": 38214896636.069016},
     {"base": 51150.0, "pointe": 33500, "semibase_winter": 0.0}
     ),
    (ALL_STUDIES_PATH / "empty-link-profile",
     {"gap": 0.0098419189453125, "investment_cost": 27585000000.000114,
      "operational_cost": 10629896636.068903, "overall_cost": 38214896636.069016},
     {"base": 51150.0, "pointe": 33500, "semibase_empty": 0.0}
     )
]


@pytest.mark.parametrize(medium_parameters_names, medium_paramters_values, )
@pytest.mark.medium_sequential
def test_full_study_medium_sequential(install_dir, study_path, expected_values, expected_investment_solution):
    verify_full_study(install_dir, study_path, expected_values, expected_investment_solution, "sequential")


@pytest.mark.parametrize(medium_parameters_names, medium_paramters_values, )
@pytest.mark.medium_mpi
def test_full_study_medium_parallel(install_dir, study_path, expected_values, expected_investment_solution):
    verify_full_study(install_dir, study_path, expected_values, expected_investment_solution, "mpibenders")
