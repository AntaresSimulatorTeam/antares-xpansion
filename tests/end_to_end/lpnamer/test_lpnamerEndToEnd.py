import filecmp
import os
import shutil
import subprocess
import pytest
from pathlib import Path

DATA_TEST = Path("../../../data_test/")
TEST_LP_01 = DATA_TEST / "tests_lpnamer" / "test_lpnamer_01" / "output" / "20210713-1635eco-new/"
TEST_LP_02 = DATA_TEST / "tests_lpnamer" / "test_one_link_one_candidate" / "output" / "20210720-1024eco/"


@pytest.fixture
def setup_and_teardown_lp_directory(request):
    test_dir = request.getfixturevalue('test_dir')
    lp_dir = test_dir / "lp"
    lp_dir.mkdir()
    yield
    shutil.rmtree(lp_dir)


@pytest.mark.parametrize("test_dir", [TEST_LP_01, TEST_LP_02])
def test_lp_namer_01(install_dir, test_dir, setup_and_teardown_lp_directory):
    # given
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir / "lp"
    
    lp_namer_exe = Path(install_dir) / "lp_namer"
    os.chdir(test_dir.parent)

    launch_command = [str(lp_namer_exe), "-o", str(test_dir.name), "-e", "contraintes.txt", "-f", "integer"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)

    # then
    os.chdir(old_path)
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0