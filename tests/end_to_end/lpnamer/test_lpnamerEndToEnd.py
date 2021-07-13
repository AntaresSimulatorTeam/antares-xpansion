import filecmp
import os
import shutil
import subprocess
from pathlib import Path

DATA_TEST = Path("../../../data_test/")
TEST_LP_01 = DATA_TEST / "test_lpnamer_01" / "output" / "20210713-1635eco-new/"


def test_lp_namer_01(installDir):
    # given
    reference_lp_dir = TEST_LP_01/"reference_lp"
    lp_dir = TEST_LP_01/"lp"
    lp_dir.mkdir()
    lp_namer_exe = Path(installDir) / "lp_namer"
    os.chdir(TEST_LP_01.parent)
    launch_command = [str(lp_namer_exe), "-o", str(TEST_LP_01.name), "-e", "contraintes.txt"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0
    # teardown
    shutil.rmtree(lp_dir)

