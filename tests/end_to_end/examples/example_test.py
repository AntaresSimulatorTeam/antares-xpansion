import shutil
import subprocess
import sys
from enum import Enum
from pathlib import Path

import numpy as np
import pytest

from src.python.antares_xpansion.candidates_reader import CandidatesReader
from tests.end_to_end.utils_functions import read_outputs, remove_outputs

ALL_STUDIES_PATH = Path("../../../data_test/examples")
RELATIVE_TOLERANCE = 1e-4
RELATIVE_TOLERANCE_LIGHT = 1e-2


class BendersMethod(Enum):
    BENDERS = "benders"
    BENDERS_BY_BATCH = "benders_by_batch"


def launch_xpansion(install_dir, study_path, allow_run_as_root=False, nproc: int = 4):
    # Clean study output
    remove_outputs(study_path)

    install_dir_full = str(Path(install_dir).resolve())

    command = [
        sys.executable,
        "../../../src/python/launch.py",
        "--installDir",
        install_dir_full,
        "--dataDir",
        str(study_path),
        "--step",
        "full",
        "-n",
        str(nproc),
        "--oversubscribe",
    ]
    if allow_run_as_root:
        command.append("--allow-run-as-root")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)
    output = process.communicate()
    if process.returncode != 0:
        print(output)

    # Check return value
    assert process.returncode == 0


def launch_xpansion_memory(install_dir, study_path, method: BendersMethod, allow_run_as_root=False, nproc: int = 4):
    # Clean study output
    remove_outputs(study_path)

    install_dir_full = str(Path(install_dir).resolve())

    command = [
        sys.executable,
        "../../../src/python/launch.py",
        "--installDir",
        install_dir_full,
        "--dataDir",
        str(study_path),
        "--method",
        method.value,
        "--step",
        "full",
        "-n",
        str(nproc),
        "--oversubscribe",
        "--memory"
    ]
    if allow_run_as_root == "True":
        command.append("--allow-run-as-root")
    print(command)
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)
    output = process.communicate()
    if process.returncode != 0:
        print(output)

    # Check return value
    assert process.returncode == 0

def assert_convergence(solution, options_data, method: BendersMethod):
    assert (solution["relative_gap"] <= options_data["RELATIVE_GAP"]) or (
        solution["overall_cost"] * solution["relative_gap"]
        <= options_data["ABSOLUTE_GAP"]) or (method == BendersMethod.BENDERS_BY_BATCH and solution["ABSOLUTE_GAP"] <= options_data["ABSOLUTE_GAP"])


def verify_solution(study_path, expected_values, expected_investment_solution,
                    method: BendersMethod = BendersMethod.BENDERS, use_archive=True):
    output_path = study_path / "output"
    outputs = read_outputs(output_path, use_archive)
    json_data = outputs.out_json
    options_data = outputs.options_json

    solution = json_data["solution"]
    investment_solution = solution["values"]

    np.testing.assert_allclose(
        solution["investment_cost"],
        expected_values["investment_cost"],
        rtol=RELATIVE_TOLERANCE_LIGHT,
    )
    np.testing.assert_allclose(
        solution["operational_cost"],
        expected_values["operational_cost"],
        rtol=RELATIVE_TOLERANCE_LIGHT,
    )
    np.testing.assert_allclose(
        solution["overall_cost"],
        expected_values["overall_cost"],
        rtol=RELATIVE_TOLERANCE,
    )

    assert_convergence(solution, options_data, method)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), (
            "Investment " + investment + " not found in solution"
        )
        np.testing.assert_allclose(
            expected_investment_solution[investment],
            investment_solution[investment],
            rtol=RELATIVE_TOLERANCE_LIGHT,
        )


def verify_study_update(study_path, expected_investment_solution, antares_version=700):
    candidate_reader = CandidatesReader(
        study_path / "user" / "expansion" / "candidates.ini"
    )
    for link in candidate_reader.get_link_list():
        candidate_name_list = candidate_reader.get_link_candidate(link)
        already_installed_direct_capacity = (
            candidate_reader.get_candidate_already_install_capacity(
                candidate_name_list[0]
            )
        )
        already_installed_indirect_capacity = (
            candidate_reader.get_candidate_already_install_capacity(
                candidate_name_list[0]
            )
        )
        already_installed_direct_link_profile_array = (
            candidate_reader.get_candidate_already_installed_direct_link_profile_array(
                study_path, candidate_name_list[0]
            )
        )
        expected_direct_link_capacity = (
            already_installed_direct_capacity
            * already_installed_direct_link_profile_array
        )
        already_installed_indirect_link_profile_array = candidate_reader.get_candidate_already_installed_indirect_link_profile_array(
            study_path, candidate_name_list[0]
        )
        expected_indirect_link_capacity = (
            already_installed_indirect_capacity
            * already_installed_indirect_link_profile_array
        )

        for candidate in candidate_name_list:
            investment = expected_investment_solution[candidate]
            (
                profile_exists,
                link_profile_array,
            ) = candidate_reader.get_candidate_link_profile_array(study_path, candidate)
            if link_profile_array.ndim == 2:
                assert (
                    link_profile_array.shape
                    == candidate_reader.get_candidate_already_installed_link_profile_array(
                        study_path, candidate_name_list[0]
                    ).shape
                )
                expected_direct_link_capacity += investment * \
                    link_profile_array[:, 0]
                expected_indirect_link_capacity += investment * \
                    link_profile_array[:, 1]
            else:
                direct_array = link_profile_array[:, :, 0].transpose()
                indirect_array = link_profile_array[:, :, 1].transpose()
                if candidate_reader.has_installed_profile(
                    study_path, candidate
                ) and candidate_reader.has_profile(study_path, candidate):
                    assert (
                        direct_array.shape
                        == already_installed_direct_link_profile_array.shape
                    )
                    assert (
                        indirect_array.shape
                        == already_installed_indirect_link_profile_array.shape
                    )
                if candidate_reader.has_profile(
                    study_path, candidate
                ) and not candidate_reader.has_installed_profile(study_path, candidate):
                    (
                        expected_direct_link_capacity,
                        expected_indirect_link_capacity,
                    ) = grow_expectation_to_proper_number_of_chronicles(
                        direct_array,
                        expected_direct_link_capacity,
                        expected_indirect_link_capacity,
                    )
                expected_direct_link_capacity += investment * direct_array
                expected_indirect_link_capacity += investment * indirect_array

        if antares_version < 820:
            assert_ntc_update_pre_820(
                candidate_reader,
                expected_direct_link_capacity,
                expected_indirect_link_capacity,
                link,
                study_path,
            )
        else:
            assert_ntc_update_post_820(
                candidate_reader,
                expected_direct_link_capacity,
                expected_indirect_link_capacity,
                link,
                study_path,
            )


def grow_expectation_to_proper_number_of_chronicles(
    direct_array, expected_direct_link_capacity, expected_indirect_link_capacity
):
    new_direct_expected_array = np.ones(direct_array.transpose().shape)
    new_indirect_expected_array = np.ones(direct_array.transpose().shape)
    for k in range(new_direct_expected_array.shape[0]):
        new_direct_expected_array[k] = expected_direct_link_capacity
        new_indirect_expected_array[k] = expected_indirect_link_capacity
    expected_direct_link_capacity = new_direct_expected_array.transpose()
    expected_indirect_link_capacity = new_indirect_expected_array.transpose()
    return expected_direct_link_capacity, expected_indirect_link_capacity


def assert_ntc_update_post_820(
    candidate_reader,
    expected_direct_link_capacity,
    expected_indirect_link_capacity,
    link,
    study_path,
):
    direct_ntc = candidate_reader.get_link_antares_direct_link_file(
        study_path, link)
    indirect_ntc = candidate_reader.get_link_antares_indirect_link_file(
        study_path, link
    )
    direct_ntc_array = np.loadtxt(direct_ntc, delimiter="\t")
    indirect_ntc_array = np.loadtxt(indirect_ntc, delimiter="\t")
    np.testing.assert_allclose(
        direct_ntc_array, expected_direct_link_capacity, rtol=RELATIVE_TOLERANCE_LIGHT
    )
    np.testing.assert_allclose(
        indirect_ntc_array,
        expected_indirect_link_capacity,
        rtol=RELATIVE_TOLERANCE_LIGHT,
    )


def assert_ntc_update_pre_820(
    candidate_reader,
    expected_direct_link_capacity,
    expected_indirect_link_capacity,
    link,
    study_path,
):
    study_link = candidate_reader.get_link_antares_link_file_pre820(
        study_path, link)
    study_link_array = np.loadtxt(study_link, delimiter="\t")
    link_capacity = study_link_array[:, [0, 1]]
    np.testing.assert_allclose(
        link_capacity,
        np.array(
            [expected_direct_link_capacity, expected_indirect_link_capacity]
        ).transpose(),
        rtol=RELATIVE_TOLERANCE_LIGHT,
    )


## TESTS ##
parameters_names = (
    "study_path, expected_values, expected_investment_solution, antares_version"
)

long_parameters_values = [
    (
        ALL_STUDIES_PATH / "xpansion-test-02",
        {
            "optimality_gap": 0,
            "investment_cost": 289923159,
            "operational_cost": 1065480665.4781473,
            "overall_cost": 1355403824.589329,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 5.66e02,
            "peak1": 6.0e02,
            "peak2": 1.0e03,
            "pv": 4.4267733994e02,
            "semibase1": 6.0e02,
            "semibase2": 0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-02-new",
        {
            "optimality_gap": 709.42435598373413,
            "investment_cost": 256299462.08039832,
            "operational_cost": 1067318031.6309935,
            "overall_cost": 1323617493.7113919,
            "relative_gap": 5.3597384391960929e-07,
            "accepted_rel_gap_atol": 5e-7,
        },
        {
            "battery": 508.74183466608417,
            "peak1": 800.0,
            "peak2": 1000.0,
            "pv": 447.28156522682048,
            "semibase1": 0.0,
            "semibase2": 200.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "different_NTCs",
        {
            "optimality_gap": 709.42435598373413,
            "investment_cost": 256299462.08039832,
            "operational_cost": 1067318031.6309935,
            "overall_cost": 1323617493.7113919,
            "relative_gap": 5.3597384391960929e-07,
            "accepted_rel_gap_atol": 5e-7,
        },
        {
            "battery": 508.74183466608417,
            "peak1": 800.0,
            "peak2": 1000.0,
            "pv": 447.28156522682048,
            "semibase1": 0.0,
            "semibase2": 200.0,
        },
        820,
    ),
]


@pytest.mark.parametrize(
    parameters_names,
    long_parameters_values,
)
@pytest.mark.long_sequential
def test_full_study_long_sequential(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study,
                    allow_run_as_root, 1)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    long_parameters_values,
)
@pytest.mark.long_mpi
def test_full_study_long_mpi(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    long_parameters_values,
)
@pytest.mark.long_benders_by_batch_mpi
def test_full_study_long_benders_by_batch_parallel(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    shutil.move(tmp_study/"user"/"expansion"/"settings_by_batch.ini",
                tmp_study/"user"/"expansion"/"settings.ini")
    method = BendersMethod.BENDERS_BY_BATCH
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values,
                    expected_investment_solution, method)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


medium_parameters_values = [
    (
        ALL_STUDIES_PATH / "xpansion-test-01",
        {
            "optimality_gap": 0,
            "investment_cost": 224600000.00003052,
            "operational_cost": 22513655727.899261,
            "overall_cost": 22738255727.899292,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 1.0e03,
            "peak": 1.4e03,
            "pv": 1.0e03,
            "semibase": 2.0e02,
            "transmission_line": 0.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-03",
        {
            "optimality_gap": 0,
            "investment_cost": 201999999.99999976,
            "operational_cost": 1161894495.7433052,
            "overall_cost": 1363894495.743305,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {"peak": 2500.0, "transmission_line": 1600.0, "semibase": 400.0},
        700,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-04-mps-rounding",
        {
            "optimality_gap": 0,
            "investment_cost": 115399999.99999046,
            "operational_cost": 21944788078.597385,
            "overall_cost": 22060188078.597374,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 1000.0000000000124,
            "peak": 0.0,
            "pv": 1000.0,
            "semibase": 0.0,
            "transmission_line": 0.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-01-weights",
        {
            "optimality_gap": 0,
            "investment_cost": 230600000.00001144,
            "operational_cost": 24001577891.450439,
            "overall_cost": 24232177891.450451,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 1000.0,
            "peak": 1500.0,
            "pv": 1000.0,
            "semibase": 200.0,
            "transmission_line": 0.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-01-hurdles-cost",
        {
            "optimality_gap": 0,
            "investment_cost": 224599999.99996948,
            "operational_cost": 22516674015.060184,
            "overall_cost": 22741274015.060154,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 1.0e03,
            "peak": 1.4e03,
            "pv": 1.0e03,
            "semibase": 2.0e02,
            "transmission_line": 0.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "additionnal-constraints",
        {
            "optimality_gap": 0,
            "investment_cost": 224600000.00003052,
            "operational_cost": 22513655727.899261,
            "overall_cost": 22738255727.899292,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 1.0e03,
            "peak": 1.4e03,
            "pv": 1.0e03,
            "semibase": 2.0e02,
            "transmission_line": 0.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "additionnal-constraints-binary",
        {
            "optimality_gap": 0,
            "investment_cost": 220499999.99999809,
            "operational_cost": 13501512886.702877,
            "overall_cost": 13722012886.702875,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {
            "battery": 885,
            "peak": 1800,
            "pv": 1.0e03,
            "semibase": 0.0,
            "transmission_line": 400.0,
        },
        700,
    ),
    (
        ALL_STUDIES_PATH / "hurdles-cost-profile-value-over-one",
        {
            "optimality_gap": 90577,
            "investment_cost": 224000000,
            "operational_cost": 1097431246.5454886,
            "overall_cost": 1321431246.5454886,
            "relative_gap": 6.8545027035272501e-05,
            "accepted_rel_gap_atol": 1e-7,
        },
        {"peak": 1500.0, "semibase": 1400.0, "transmission_line": 800.0},
        700,
    ),
]


@pytest.mark.parametrize(
    parameters_names,
    medium_parameters_values,
)
@pytest.mark.medium_sequential
def test_full_study_medium_sequential(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study, allow_run_as_root, 1)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    medium_parameters_values,
)
@pytest.mark.medium_mpi
def test_full_study_medium_parallel(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    medium_parameters_values,
)
@pytest.mark.medium_benders_by_batch_mpi
def test_full_study_medium_benders_by_batch_parallel(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    shutil.move(tmp_study/"user"/"expansion"/"settings_by_batch.ini",
                tmp_study/"user"/"expansion"/"settings.ini")
    method = BendersMethod.BENDERS_BY_BATCH
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values,
                    expected_investment_solution, method)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


short_parameters_values = [
    (
        ALL_STUDIES_PATH / "link-profile-with-empty-week",
        {
            "optimality_gap": 0,
            "investment_cost": 27585000000.000004,
            "operational_cost": 10629896636.069275,
            "overall_cost": 38214896636.069275,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {"base": 51150.0, "pointe": 33500, "semibase_winter": 0.0},
        800,
    ),
    (
        ALL_STUDIES_PATH / "empty-link-profile",
        {
            "optimality_gap": 0,
            "investment_cost": 27584999999.999992,
            "operational_cost": 10629896636.069145,
            "overall_cost": 38214896636.069138,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {"base": 51150.0, "pointe": 33500, "semibase_empty": 0.0},
        800,
    ),
    (
        ALL_STUDIES_PATH / "xpansion-test-one-link-two-candidates",
        {
            "optimality_gap": 0,
            "investment_cost": 15999999.999994278,
            "operational_cost": 11800333780.786646,
            "overall_cost": 11816333780.78664,
            "relative_gap": 0,
            "accepted_rel_gap_atol": 1e-10,
        },
        {"transmission_line": 800.0, "transmission_line_2": 800.0},
        800,
    ),
]


@pytest.mark.parametrize(
    parameters_names,
    short_parameters_values,
)
@pytest.mark.short_sequential
def test_full_study_short_sequential(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study,
                    allow_run_as_root, nproc=1)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    short_parameters_values,
)
@pytest.mark.short_memory
def test_full_study_short_memory(
        install_dir,
        allow_run_as_root,
        study_path,
        expected_values,
        expected_investment_solution,
        tmp_path,
        antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion_memory(install_dir, tmp_study, BendersMethod.BENDERS,
                           allow_run_as_root, nproc=1)
    verify_solution(tmp_study, expected_values, expected_investment_solution, use_archive=False)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    short_parameters_values,
)
@pytest.mark.short_mpi
def test_full_study_short_parallel(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values, expected_investment_solution)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)


@pytest.mark.parametrize(
    parameters_names,
    short_parameters_values,
)
@pytest.mark.short_benders_by_batch_mpi
def test_full_study_short_benders_by_batch_parallel(
    install_dir,
    allow_run_as_root,
    study_path,
    expected_values,
    expected_investment_solution,
    tmp_path,
    antares_version,
):
    tmp_study = tmp_path / study_path.name
    shutil.copytree(study_path, tmp_study)
    shutil.move(tmp_study/"user"/"expansion"/"settings_by_batch.ini",
                tmp_study/"user"/"expansion"/"settings.ini")
    method = BendersMethod.BENDERS_BY_BATCH
    launch_xpansion(install_dir, tmp_study, allow_run_as_root)
    verify_solution(tmp_study, expected_values,
                    expected_investment_solution, method)
    verify_study_update(
        tmp_study, expected_investment_solution, antares_version)
