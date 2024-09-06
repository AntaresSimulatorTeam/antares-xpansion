import json
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


def build_outer_loop_command(context):
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=context.nproc)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf("OUTER_LOOP")
    command.append(str(exe_path))
    command.append("options.json")
    return command



def read_outputs(output_path):
    with open(output_path, 'r') as file:
        outputs = json.load(file)

    return outputs


@when('I run outer loop with {n} proc(s)')
def run_outer_loop(context, n):
    context.nproc = int(n)
    context.allow_run_as_root = get_conf("allow_run_as_root")
    command = build_outer_loop_command(context)
    print(f"Running command: {command}")
    old_cwd = os.getcwd()
    lp_path = Path(context.study_path) / "lp"

    os.chdir(lp_path)
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    out, err = process.communicate()
    # print(out)
    # print("*****")
    # print(err)
    context.return_code = process.returncode
    context.outputs = read_outputs(Path("..") / "expansion" / "out.json")
    os.chdir(old_cwd)


@then("the simulation takes less than {seconds} seconds")
def check_simu_time(context, seconds):
    assert context.outputs["run_duration"] <= float(seconds)


@then("the simulation succeeds")
def simu_success(context):
    return context.return_code == 0


@then("the expected overall cost is {value}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(float(value),
                               context.outputs["solution"]["overall_cost"], rtol=1e-6, atol=0)


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
