import filecmp
import os
import shutil
import subprocess
import zipfile
from pathlib import Path

import pytest

MPS_ZIP = "economy.zip"
DATA_TEST = Path("../../../data_test/")
DATA_TEST_INTEGER = DATA_TEST / "tests_lpnamer" / "tests_integer"
DATA_TEST_RELAXED = DATA_TEST / "tests_lpnamer" / "tests_relaxed"
TEST_LP_INTEGER_01 = DATA_TEST_INTEGER / \
    "test_lpnamer_01" / "output" / "economy"
TEST_LP_INTEGER_02 = DATA_TEST_INTEGER / \
    "test_one_link_one_candidate_1week" / "output" / "economy/"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob" \
    / "output" / "economy"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES = DATA_TEST_INTEGER / \
    "test_one_link_two_candidates_1week" / "output" / "economy"

TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_HURDLES = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob_hurdle_cost" \
    / "output" / "economy"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_NULL_PROFILE = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob_null_profile" \
    / "output" / "economy"

TEST_LP_RELAXED_01 = DATA_TEST_RELAXED / \
    "test_one_link_one_candidate-relaxed" / "output" / "economy/"
TEST_LP_RELAXED_02 = DATA_TEST_RELAXED / "SmallTestSixCandidatesWithAlreadyInstalledCapacity-relaxed" / "output" \
    / "economy"
test_data = [
    (TEST_LP_INTEGER_01, "integer", True),
    (TEST_LP_INTEGER_01, "integer", False),
    (TEST_LP_INTEGER_02, "integer", True),
    (TEST_LP_INTEGER_02, "integer", False),
    (TEST_LP_RELAXED_01, "relaxed", True),
    (TEST_LP_RELAXED_01, "relaxed", False),
    (TEST_LP_RELAXED_02, "relaxed", True),
    (TEST_LP_RELAXED_02, "relaxed", False),
]

test_data_multiple_candidates = [
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB, "integer", True),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB, "integer", False),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES, "integer", True),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES, "integer", False),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_HURDLES, "integer", True),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_HURDLES, "integer", False),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_NULL_PROFILE, "integer", True),
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_NULL_PROFILE, "integer", False),
]


@pytest.fixture
def setup_and_teardown_lp_directory(request):
    test_dir = request.getfixturevalue('test_dir')
    lp_dir = test_dir.parent / (test_dir.stem + "-Xpansion") / "lp"
    if Path(lp_dir).is_dir():
        shutil.rmtree(lp_dir)
    Path(lp_dir).mkdir(parents=True, exist_ok=True)

    list_files = list(Path(test_dir).glob("*.mps"))
    list_files.extend(list(Path(test_dir).glob("variables*.txt")))
    list_files.extend(list(Path(test_dir).glob("area*.txt")))
    list_files.extend(list(Path(test_dir).glob("interco*.txt")))
    with zipfile.ZipFile(test_dir.parent / MPS_ZIP, "w") as write_mps_zip:
        for file in list_files:
            write_mps_zip.write(
                file, file.name, compress_type=zipfile.ZIP_DEFLATED)
    yield


@pytest.mark.parametrize("test_dir,master_mode, use_archive", test_data)
def test_lp_directory_files(install_dir, test_dir, master_mode, use_archive, setup_and_teardown_lp_directory):
    # given
    if use_archive:
        launch_and_compare_lp_with_reference_archive(install_dir, master_mode, test_dir)
    else:
        launch_and_compare_lp_with_reference_output(install_dir, master_mode, test_dir)


@pytest.mark.parametrize("test_dir,master_mode, use_archive", test_data_multiple_candidates)
def test_lp_multiple_candidates(install_dir, test_dir, master_mode, use_archive, setup_and_teardown_lp_directory):
    if use_archive:
        launch_and_compare_lp_with_reference_archive(install_dir, master_mode, test_dir)
    else:
        launch_and_compare_lp_with_reference_output(install_dir, master_mode, test_dir)


def launch_and_compare_lp_with_reference_output(install_dir, master_mode, test_dir):
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir / "lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    study_dir = test_dir.resolve()
    os.chdir(test_dir.parent)
    launch_command = [str(lp_namer_exe), "-o", str(study_dir),
                      "-e", "contraintes.txt", "-f", master_mode, "--unnamed-problems"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    os.chdir(old_path)
    then(lp_dir, old_path, reference_lp_dir, returned_l)



def launch_and_compare_lp_with_reference_archive(install_dir, master_mode, test_dir):
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir.parent / (test_dir.stem + "-Xpansion") / "lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    zip_path = (test_dir.parent / MPS_ZIP).resolve()
    os.chdir(test_dir.parent)
    launch_command = [str(lp_namer_exe), "-a", str(zip_path),
                      "-e", "contraintes.txt", "-f", master_mode, "--unnamed-problems"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    then(lp_dir, old_path, reference_lp_dir, returned_l)


def then(lp_dir, old_path, reference_lp_dir, returned_l):
    os.chdir(old_path)
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(
        reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0
