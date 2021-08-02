import filecmp
import os
import shutil
import subprocess
import pytest
from pathlib import Path

DATA_TEST = Path("../../../data_test/")
TEST_LP_INTEGER_01 = DATA_TEST / "tests_lpnamer" / "tests_integer" / "test_lpnamer_01" / "output" \
                     / "economy/"
TEST_LP_INTEGER_02 = DATA_TEST / "tests_lpnamer" / "tests_integer" / "test_one_link_one_candidate_1week" / "output" \
                     / "20210720-1024eco/"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES = DATA_TEST / "tests_lpnamer" / "tests_integer" \
                                      / "test_one_link_two_candidates_1week_MR" \
                                      / "output" / "20210721-1451eco"
TEST_LP_RELAXED_01 = DATA_TEST / "tests_lpnamer" / "tests_relaxed" / "test_one_link_one_candidate-relaxed" / "output" \
                     / "20210720-1147eco/"
TEST_LP_RELAXED_02 = DATA_TEST / "tests_lpnamer" / "tests_relaxed" \
                     / "SmallTestSixCandidatesWithAlreadyInstalledCapacity-relaxed" / "output" / "20210720-1433eco/"

test_data = [
    (TEST_LP_INTEGER_01, "integer"),
    (TEST_LP_INTEGER_02, "integer"),
    (TEST_LP_RELAXED_01, "relaxed"),
    (TEST_LP_RELAXED_02, "relaxed")
]

test_data_multiple_candidates = [
    (TEST_LP_INTEGER_MULTIPLE_CANDIDATES, "integer")
]


@pytest.fixture
def setup_and_teardown_lp_directory(request):
    test_dir = request.getfixturevalue('test_dir')
    lp_dir = test_dir / "lp"
    if Path(lp_dir).is_dir():
        shutil.rmtree(lp_dir)
    Path(lp_dir).mkdir(exist_ok=True)
    yield
    shutil.rmtree(lp_dir)


@pytest.mark.parametrize("test_dir,master_mode", test_data)
def test_lp_directory_files(install_dir, test_dir, master_mode, setup_and_teardown_lp_directory):
    # given
    launch_and_compare_lp_with_reference(install_dir, master_mode, test_dir)


@pytest.mark.parametrize("test_dir,master_mode", test_data_multiple_candidates)
def test_lp_multiple_candidates(install_dir, test_dir, master_mode, setup_and_teardown_lp_directory):
    launch_and_compare_lp_with_reference(install_dir, master_mode, test_dir)


def launch_and_compare_lp_with_reference(install_dir, master_mode, test_dir):
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir / "lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    os.chdir(test_dir.parent)
    launch_command = [str(lp_namer_exe), "-o", str(test_dir.name), "-e", "contraintes.txt", "-f", master_mode]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    os.chdir(old_path)
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0
