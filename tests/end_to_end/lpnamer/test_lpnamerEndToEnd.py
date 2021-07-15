import filecmp
import os
import shutil
import subprocess
import pytest
from pathlib import Path

DATA_TEST = Path("../../../data_test/")
TEST_LP_01 = DATA_TEST / "tests_lpnamer" / "test_lpnamer_01" / "output" / "20210713-1635eco-new/"


@pytest.fixture
def setup_and_teardown_lp_directory(request):
    testDir = request._fixture_defs['testDir'].params[0]
    lp_dir = testDir / "lp"
    lp_dir.mkdir()
    yield
    shutil.rmtree(lp_dir)


@pytest.mark.parametrize("testDir", [TEST_LP_01])
def test_lp_namer_01(installDir, testDir, setup_and_teardown_lp_directory):
    # given
    old_path = os.getcwd()
    reference_lp_dir = testDir / "reference_lp"
    lp_dir = testDir / "lp"
    
    lp_namer_exe = Path(installDir) / "lp_namer"
    os.chdir(testDir.parent)
    launch_command = [str(lp_namer_exe), "-o", str(testDir.name), "-e", "contraintes.txt"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    os.chdir(old_path)
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0
