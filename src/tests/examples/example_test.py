from pathlib import Path
import urllib.request
import zipfile
import os
import sys
import glob
import shutil

import numpy as np
import subprocess

import pytest

ALL_STUDIES_PATH = Path('../../../examples')


def find_log_path(output_dir):
    op = []
    for path in Path(output_dir).iterdir():
        for log in Path(path / "lp").rglob('*.log'):
            op.append(log)
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

    command = [sys.executable, "../../src_python/launch.py", "--installDir", install_dir_full, "--dataDir", str(study_path),"--method", method, "--step","full"]
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)
    output = process.communicate()

    # Check return value
    assert process.returncode == 0


def check_investment_solution(study_path, expected_last_it,expected_investment_solution):
    output_path = study_path / 'output'
    log_path = find_log_path(output_path)

    log_file = open(str(log_path), 'r')
    log = log_file.readline()

    last_it = []
    investment_solution = {}

    # search in log content for iteration result and investment solution
    while log:
        if "ITE" in log and "LB" in log and "UB" in log:
            it = log_file.readline().strip()
            while len(it) != 0:
                last_it = [float(i) for i in it.split()]
                it = log_file.readline().strip()
        if "Investment solution" in log:
            res = log_file.readline().strip()
            while len(res) != 0:
                if ("=" in res):
                    investment_solution_list = res.replace("|", "").strip().split("=")
                    assert (len(investment_solution_list) == 2)
                    investment_solution[investment_solution_list[0].strip()] = float(investment_solution_list[1])
                res = log_file.readline().strip()
        log = log_file.readline()

    log_file.close()

    np.testing.assert_allclose(last_it[1:3], expected_last_it[0:2], rtol=1e-4, atol=0)

    for investment in expected_investment_solution.keys():
        assert investment in investment_solution.keys(), "Investment " + investment + " not found in solution"

        np.testing.assert_allclose(expected_investment_solution[investment], investment_solution[investment], rtol=1e-4,
                                   atol=0)

## TESTS ##
@pytest.mark.medium
def test_001(installDir):
    study_path = ALL_STUDIES_PATH / "xpansion-test-01"
    launch_xpansion(installDir, study_path, "sequential")

    expected_last_it = [2.2738256189e+10, 2.2738256189e+10]
    expected_investment_solution = {"battery" : 1.0e+03, "peak" : 1.4e+03 , "pv" : 1.0e+03 , "semibase" : 2.0e+02}

    check_investment_solution(study_path,expected_last_it,expected_investment_solution)
