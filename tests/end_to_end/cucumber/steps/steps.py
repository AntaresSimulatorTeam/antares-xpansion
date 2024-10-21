import json
import math
import os
import subprocess
from pathlib import Path

import numpy as np
from behave import *

from utils_functions import get_mpi_command, get_conf


@given('the study path is "{string}"')
def study_path_is(context, string):
    context.study_path = os.path.join(Path() / "../../",
                                      string.replace("/", os.sep))


def build_outer_loop_command(context, n: int, option_file: str = "options.json"):
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=n)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf("OUTER_LOOP")
    command.append(str(exe_path))
    command.append(option_file)
    return command


def build_launch_command(study_dir: str, method: str, nproc: int, in_memory: bool):
    command = f"python ../../src/python/launch.py --installDir {get_conf('DEFAULT_INSTALL_DIR')} --dataDir {study_dir} --method {method} -n {nproc} --oversubscribe"
    if in_memory:
        command += " --memory"
    return command


def read_json_file(output_path):
    with open(output_path, 'r') as file:
        outputs = json.load(file)
    return outputs


def read_file(output_path):
    with open(output_path, 'r') as file:
        outputs = file.readlines()
    return outputs


@when('I run outer loop with {n:d} proc(s) and "{option_file}" as option file')
@when('I run outer loop with {n:d} proc(s)')
def run_outer_loop(context, n, option_file: str = "options.json"):
    context.allow_run_as_root = get_conf("allow_run_as_root")
    command = build_outer_loop_command(context, n, option_file)
    print(f"Running command: {command}")
    old_cwd = os.getcwd()

    lp_path = Path(context.study_path) / "lp" if (Path(context.study_path) / "lp").exists() else Path(
        context.study_path)

    os.chdir(lp_path)
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    process.communicate()
    context.return_code = process.returncode
    options = read_json_file(option_file)
    output_file_path = options["JSON_FILE"]
    context.outputs = read_json_file(output_file_path)
    context.loss_of_load_file = (Path(options["OUTPUTROOT"]) / "LOLD.txt").absolute()
    context.positive_unsupplied_energy_file = (Path(options["OUTPUTROOT"]) / "PositiveUnsuppliedEnergy.txt").absolute()

    os.chdir(old_cwd)


@when('I run antares-xpansion in memory with the {method} method and {n:d} proc(s)')
def run_antares_xpansion(context, method, n):
    command = build_launch_command(context.study_path, method, n, True)
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, shell=True)
    out, err = process.communicate()
    context.return_code = process.returncode
    context.outputs = read_json_file(Path(get_results_file_path_from_logs(out)))


@then("the simulation takes less than {seconds:d} seconds")
def check_simu_time(context, seconds):
    assert context.outputs["run_duration"] <= seconds


@then("the simulation succeeds")
def simu_success(context):
    assert context.return_code == 0


@then("the expected overall cost is {value:g}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["overall_cost"], rtol=1e-6, atol=0)


def assert_dict_allclose(actual, expected, rtol=1e-06, atol=0):
    for key in expected:
        np.testing.assert_allclose(
            actual[key],
            expected[key],
            rtol=rtol,
            atol=atol,
            err_msg=f"Mismatch found at key '{key}'"
        )


@then("the solution is")
def check_solution(context):
    expected_solution = {row['variable']: float(row['value']) for row in context.table}
    assert_dict_allclose(context.outputs["solution"]["values"], expected_solution)


def is_column_full_of_zeros(filename, column_index, abs_tol=1e-9):
    with open(filename, 'r') as file:
        # Skip the header
        next(file)

        # Check each line in the file
        for line in file:
            columns = line.split()

            # Ensure column exists
            if column_index >= len(columns):
                print(f"Error: Missing column at index {column_index} in line: {line.strip()}")
                return False

            try:
                value = float(columns[column_index])
            except (ValueError, IndexError):
                print(f"Error parsing line: {line.strip()}")
                return False

            # Use math.isclose to compare to zero with tolerance
            if not math.isclose(value, 0.0, abs_tol=abs_tol):
                print(f"Error {value} is not close to 0")
                return False

    return True


@then("LOLD.txt and PositiveUnsuppliedEnergy.txt files are full of zeros")
def check_other_outputs(context):
    assert (is_column_full_of_zeros(context.loss_of_load_file, 2))
    assert (is_column_full_of_zeros(context.positive_unsupplied_energy_file, 2))

def get_results_file_path_from_logs(logs: bytes) -> str:
    for line in logs.splitlines():
        if b'Optimization results available in : ' in line:
            return line.split(b'Optimization results available in : ')[1].decode('ascii')
    raise LookupError("Could not find results file path in output logs")
