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

    np.testing.assert_allclose(
        solution["investment_cost"], expected_values["investment_cost"], rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(solution["operational_cost"], expected_values["operational_cost"],
                               rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(
        solution["overall_cost"], expected_values["overall_cost"], rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(
        solution["optimality_gap"], expected_values["optimality_gap"], rtol=RELATIVE_TOLERANCE)
    np.testing.assert_allclose(
        solution["relative_gap"], expected_values["relative_gap"], rtol=RELATIVE_TOLERANCE)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), "Investment " + investment + \
            " not found in solution"
        np.testing.assert_allclose(expected_investment_solution[investment], investment_solution[investment],
                                   rtol=RELATIVE_TOLERANCE)


def verify_study_update(study_path, expected_investment_solution):
    candidate_reader = CandidatesReader(
        study_path / "user" / "expansion" / "candidates.ini")
    for link in candidate_reader.get_link_list():
        candidate_name_list = candidate_reader.get_link_candidate(link)
        already_installed_capacity = candidate_reader.get_candidate_already_install_capacity(
            candidate_name_list[0])
        already_installed_link_profile_array = candidate_reader.get_candidate_already_installed_link_profile_array(
            study_path, candidate_name_list[0])
        expected_link_capacity = already_installed_capacity * \
            already_installed_link_profile_array

        for candidate in candidate_name_list:
            investment = expected_investment_solution[candidate]
            link_profile_array = candidate_reader.get_candidate_link_profile_array(
                study_path, candidate)
            assert link_profile_array.shape == already_installed_link_profile_array.shape
            expected_link_capacity += investment * link_profile_array

        study_link = candidate_reader.get_link_antares_link_file(
            study_path, link)
        study_link_array = np.loadtxt(study_link, delimiter="\t")
        link_capacity = study_link_array[:, [0, 1]]

        np.testing.assert_allclose(
            link_capacity, expected_link_capacity, rtol=1e-4)
    


## TESTS ##
long_parameters_names = "study_path, expected_values, expected_investment_solution"
long_parameters_values = [
        (
            ALL_STUDIES_PATH / "xpansion-test-02",
            {
                "optimality_gap": -1.430511474609375e-06,
                "investment_cost": 289923159.11118174,
                "operational_cost": 1065480665.4781473,
                "overall_cost": 1355403824.589329,
                "relative_gap": -1.0554134853816003e-15,
            },
            {
                "battery": 5.66e02,
                "peak1": 6.0e02,
                "peak2": 1.0e03,
                "pv": 4.4267733994e02,
                "semibase1": 6.0e02,
            },
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-02-new",
            {
                "optimality_gap": 709.42435598373413,
                "investment_cost": 256299462.08039832,
                "operational_cost": 1067318031.6309935,
                "overall_cost": 1323617493.7113919,
                "relative_gap": 5.3597384391960929e-07,
            },
            {
                "battery": 508.74183466608417,
                "peak1": 800.0,
                "peak2": 1000.0,
                "pv": 447.28156522682048,
                "semibase1": 0.0,
                "semibase2": 200.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-05-area-uppercase",
            {
                "optimality_gap": 558.44661593437195,
                "investment_cost": 451995209.27069736,
                "operational_cost": 1324857537.5020638,
                "overall_cost": 1776852746.7727611,
                "relative_gap": 3.1428975583298067e-07,
            },
            {
                "elec_grid": 709.95027341942216,
                "h2_grid": 600.01391420346658,
                "p2g_marg_area1": 1125.0003565560044,
                "p2g_marg_area2": 2000.0,
            },
        ),
    ]

@pytest.mark.parametrize(long_parameters_names, long_parameters_values)
@pytest.mark.long_sequential
def verify_full_study_long_sequential(install_dir, study_path, expected_values, expected_investment_solution):
    launch_xpansion(install_dir, study_path, "sequential")
    verify_solution(study_path, expected_values, expected_investment_solution)
    verify_study_update(study_path, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.parametrize(long_parameters_names, long_parameters_values)
@pytest.mark.long_mpi
def verify_full_study_long_mpi(install_dir, study_path, expected_values, expected_investment_solution):
    launch_xpansion(install_dir, study_path, "mpibenders")
    verify_solution(study_path, expected_values, expected_investment_solution)
    verify_study_update(study_path, expected_investment_solution)
    remove_outputs(study_path)


medium_parameters_names = "study_path, expected_values, expected_investment_solution"
medium_parameters_values = [
        (
            ALL_STUDIES_PATH / "xpansion-test-01",
            {
                "optimality_gap": 3.0517578125e-05,
                "investment_cost": 224600000.00003052,
                "operational_cost": 22513655727.899261,
                "overall_cost": 22738255727.899292,
                "relative_gap": 1.3421248529435642e-15,
            },
            {
                "battery": 1.0e03,
                "peak": 1.4e03,
                "pv": 1.0e03,
                "semibase": 2.0e02,
                "transmission_line": 0.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-03",
            {
                "optimality_gap": -1.1920928955078125e-06,
                "investment_cost": 201999999.99999976,
                "operational_cost": 1161894495.7433052,
                "overall_cost": 1363894495.743305,
                "relative_gap": -8.7403600441846301e-16,
            },
            {"peak": 2500.0, "transmission_line": 1600.0, "semibase": 400.0},
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-04-mps-rounding",
            {
                "optimality_gap": -1.1444091796875e-05,
                "investment_cost": 115399999.99999619,
                "operational_cost": 21942458064.791809,
                "overall_cost": 22057858064.791805,
                "relative_gap": -5.1882153576560404e-16,
            },
            {
                "battery": 1000.0000000000124,
                "peak": 0.0,
                "pv": 1000.0,
                "semibase": 0.0,
                "transmission_line": 0.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-01-weights",
            {
                "optimality_gap": -3.814697265625e-06,
                "investment_cost": 230600000.00001144,
                "operational_cost": 24001577891.450439,
                "overall_cost": 24232177891.450451,
                "relative_gap": -1.5742279883851852e-16,
            },
            {
                "battery": 1000.0,
                "peak": 1500.0,
                "pv": 1000.0,
                "semibase": 200.0,
                "transmission_line": 0.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-one-link-two-candidates",
            {
                "optimality_gap": -1.9073486328125e-06,
                "investment_cost": 15999999.999994278,
                "operational_cost": 11800333780.786646,
                "overall_cost": 11816333780.78664,
                "relative_gap": -1.6141627921122616e-16,
            },
            {"transmission_line": 800.0, "transmission_line_2": 800.0},
        ),
        (
            ALL_STUDIES_PATH / "xpansion-test-01-hurdles-cost",
            {
                "optimality_gap": -7.62939453125e-06,
                "investment_cost": 224599999.99996948,
                "operational_cost": 22516674015.060184,
                "overall_cost": 22741274015.060154,
                "relative_gap": -3.3548668057020544e-16,
            },
            {
                "battery": 1.0e03,
                "peak": 1.4e03,
                "pv": 1.0e03,
                "semibase": 2.0e02,
                "transmission_line": 0.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "additionnal-constraints",
            {
                "optimality_gap": 3.0517578125e-05,
                "investment_cost": 224600000.00003052,
                "operational_cost": 22513655727.899261,
                "overall_cost": 22738255727.899292,
                "relative_gap": 1.3421248529435642e-15,
            },
            {
                "battery": 1.0e03,
                "peak": 1.4e03,
                "pv": 1.0e03,
                "semibase": 2.0e02,
                "transmission_line": 0.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "additionnal-constraints-binary",
            {
                "optimality_gap": -7.62939453125e-06,
                "investment_cost": 220499999.99999809,
                "operational_cost": 13501512886.702877,
                "overall_cost": 13722012886.702875,
                "relative_gap": -5.5599674728794044e-16,
            },
            {
                "battery": 885,
                "peak": 1800,
                "pv": 1.0e03,
                "semibase": 0.0,
                "transmission_line": 400.0,
            },
        ),
        (
            ALL_STUDIES_PATH / "hurdles-cost-profile-value-over-one",
            {
                "optimality_gap": 239316.41654706001,
                "investment_cost": 231999999.99999976,
                "operational_cost": 1089603112.1225781,
                "overall_cost": 1321603112.1225779,
                "relative_gap": 0.00018108039724778097,
            },
            {"peak": 1300.0, "semibase": 1600.0, "transmission_line": 1000.0},
        ),
        (
            ALL_STUDIES_PATH / "link-profile-with-empty-week",
            {
                "optimality_gap": -7.62939453125e-06,
                "investment_cost": 27585000000.000004,
                "operational_cost": 10629896636.069275,
                "overall_cost": 38214896636.069275,
                "relative_gap": -1.9964451569519531e-16,
            },
            {"base": 51150.0, "pointe": 33500, "semibase_winter": 0.0},
        ),
        (
            ALL_STUDIES_PATH / "empty-link-profile",
            {
                "optimality_gap": -0.0001373291015625,
                "investment_cost": 27584999999.999992,
                "operational_cost": 10629896636.069145,
                "overall_cost": 38214896636.069138,
                "relative_gap": -3.5936012825135282e-15,
            },
            {"base": 51150.0, "pointe": 33500, "semibase_empty": 0.0},
        ),
    ]


@pytest.mark.parametrize(medium_parameters_names, medium_parameters_values)
@pytest.mark.medium_sequential
def verify_full_study_medium_sequential(install_dir, study_path, expected_values, expected_investment_solution):
    launch_xpansion(install_dir, study_path, "sequential")
    verify_solution(study_path, expected_values, expected_investment_solution)
    verify_study_update(study_path, expected_investment_solution)
    remove_outputs(study_path)


@pytest.mark.parametrize(medium_parameters_names, medium_parameters_values)
@pytest.mark.medium_mpi
def verify_full_study_medium_parallel(install_dir, study_path, expected_values, expected_investment_solution):
    launch_xpansion(install_dir, study_path, "mpibenders")
    verify_solution(study_path, expected_values, expected_investment_solution)
    verify_study_update(study_path, expected_investment_solution)
    remove_outputs(study_path)
