import csv
import fileinput
import glob
import io
import json
import math
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from behave import *
from utils_functions import get_mpi_command, get_conf, read_outputs, remove_outputs, find_in_simulator_log


def analyze_mps_file(mps_file_path):
    """Analyze an MPS file to count rows, columns, and elements"""
    rows = 0
    cols = 0
    elements = 0

    with open(mps_file_path, 'r') as f:
        in_rows = False
        in_columns = False
        col_names = set()

        for line in f:
            line = line.strip()
            if not line:
                continue

            if line.startswith('ROWS'):
                in_rows = True
                in_columns = False
                continue
            elif line.startswith('COLUMNS'):
                in_rows = False
                in_columns = True
                continue
            elif line.startswith('RHS') or line.startswith('BOUNDS') or line.startswith('ENDATA'):
                in_rows = False
                in_columns = False
                continue

            if in_rows and not line.startswith('*'):
                rows += 1
            elif in_columns and not line.startswith('*'):
                parts = line.split()
                if len(parts) >= 1:
                    col_names.add(parts[0])
                    elements += (len(parts) - 1) // 2  # Each pair after column name is a constraint

        cols = len(col_names)

    return rows, cols, elements


def get_subproblem_statistics(lp_path):
    """Get statistics for all subproblems in the lp directory"""
    mps_files = glob.glob(str(lp_path / "**/*.mps"), recursive=True)

    if not mps_files:
        return 0, 0, 0

    total_rows = 0
    total_cols = 0
    total_elements = 0

    for mps_file in mps_files:
        if "master" not in mps_file.lower():  # Skip master problem files
            rows, cols, elements = analyze_mps_file(mps_file)
            total_rows += rows
            total_cols += cols
            total_elements += elements

    # Return average per subproblem
    num_subproblems = len([f for f in mps_files if "master" not in f.lower()])
    if num_subproblems > 0:
        return total_rows // num_subproblems, total_cols // num_subproblems, total_elements // num_subproblems
    else:
        return 0, 0, 0


@given('the study path is "{string}"')
def study_path_is(context, string):
    # context.study_path
    context.study_path = Path() / "../../" / string
    context.tmp_study = context.temp_dir / context.study_path.name
    shutil.copytree(context.study_path, context.tmp_study)


@given('solver is "{string}"')
def set_solver(context, string):
    context.solver = string
    match = False
    with fileinput.FileInput(str(context.tmp_study / "user" / "expansion" / "settings.ini"), inplace=True) as file:
        for line in file:
            match = match or re.search(r'solver\s*=.*', line)
            print(re.sub(r'solver\s*=.*', f'solver= {string}', line), end='')
    if not match:
        with open(context.tmp_study / "user/expansion/settings.ini", 'a') as file:
            file.write(f'\nsolver = {string}')


def build_outer_loop_command(context, n: int, option_file: str = "options.json"):
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=n)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf("OUTER_LOOP")
    command.append(str(exe_path))
    command.append(option_file)
    return command


def build_launch_command(study_dir: Path, step: str = None, nproc: int = 2, in_memory: bool = False,
                         method: str = None, allow_run_as_root: bool = False):
    command = [
        sys.executable,
        "../../src/python/launch.py",
        "--installDir", str(get_conf('DEFAULT_INSTALL_DIR')),
        "--dataDir", str(study_dir),
        "-n", str(nproc),
        "--oversubscribe"
    ]

    # Only add --method if method is provided and not None
    if method is not None:
        command.extend(["--method", method])

    if step is not None:
        command.extend(["--step", step])

    if in_memory:
        command.append("--memory")
    if allow_run_as_root:
        command.append("--allow-run-as-root")
    return command


def read_json_file(output_path):
    with open(output_path, 'r') as file:
        outputs = json.load(file)
    return outputs


@when('I run outer loop with {n:d} proc(s) and "{option_file}" as option file')
@when('I run outer loop with {n:d} proc(s)')
def run_outer_loop(context, n, option_file: str = "options.json"):
    context.allow_run_as_root = get_conf("allow_run_as_root")
    command = build_outer_loop_command(context, n, option_file)
    print(f"Running command: {' '.join(command)}")
    old_cwd = os.getcwd()

    lp_path = Path(context.tmp_study) / "lp" if (Path(context.tmp_study) / "lp").exists() else Path(
        context.tmp_study)

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


@when('I run antares-xpansion with the {method} method and {n:d} proc(s)')
@when('I run antares-xpansion in {memory} with the {method} method and {n:d} proc(s)')
def run_antares_xpansion(context, method, memory=None, n: int = 1):
    memory = True if memory is not None else False
    # Clean study output
    remove_outputs(context.tmp_study)

    context.return_code = run_command(context.tmp_study, memory=memory, method=method, n_mpi=n,
                                      allow_run_as_root=get_conf("allow_run_as_root"))
    if context.return_code == 0:  # If the simulation failed we're not sur outputs have been generated properly
        output_path = context.tmp_study / "output"
        outputs = read_outputs(output_path, use_archive=not memory, lold=True, positive_unsupplied_energy=True)
        context.outputs = outputs.out_json
        context.options_data = outputs.options_json
        context.lold = outputs.lold
        context.positive_unsupplied_energy = outputs.positive_unsupplied_energy


def run_command(study_path, memory, method, n_mpi, allow_run_as_root=False):
    command = build_launch_command(study_path, method=method, nproc=n_mpi, in_memory=memory,
                                   allow_run_as_root=allow_run_as_root)
    print(f"Running command: {' '.join(command)}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    print(f"Process finished with code: {process.returncode}")
    if process.returncode != 0:
        print("*********************** Begin stdout ***********************")
        print(out.replace(b'\r\n', b'\n').decode('utf-8'))
        print("*********************** End stdout ***********************")

        print("*********************** Begin stderr ***********************")
        print(err.replace(b'\r\n', b'\n').decode('utf-8'))
        print("*********************** End stderr ***********************")

    return process.returncode


@then("the simulation takes less than {seconds:d} seconds")
def check_simu_time(context, seconds):
    assert context.outputs["run_duration"] <= seconds


@then("the simulation succeeds")
def simu_success(context):
    assert context.return_code == 0


@then("the expected overall cost is {value:g}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["overall_cost"], rtol=1e-6, atol=0)


@then("the expected investment cost is {value:g}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["investment_cost"], rtol=1e-6, atol=0)


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


def read_table_from_string(raw_data):
    reader = csv.reader(io.StringIO(raw_data), delimiter='\t')
    header = [item.strip() for item in
              next(reader)[1:-1]]  # Store the header row, ignoring the first and last columns
    current_results = [{header[index]: item.strip() for index, item in enumerate(row[1:-1])} for row in
                       reader]

    return current_results


def read_cucumber_table_from_file(filename):
    with open(filename, 'r') as file:
        reader = csv.reader(file, delimiter='\t')
        header = [item.strip() for item in
                  next(reader)[1:-1]]  # Store the header row, ignoring the first and last columns
        current_results = [{header[index]: item.strip() for index, item in enumerate(row[1:-1])} for row in
                           reader]
        return current_results


def is_file_full_of_zeros(filename, abs_tol=1e-9):
    data = read_cucumber_table_from_file(filename)

    for line_number, line in enumerate(data):

        for key, value in line.items():
            if key in ["Outer loop", "Ite"]:
                continue
            try:
                value = float(value)
            except (ValueError, IndexError):
                print(f"Error parsing line: {line_number} at column {key}")
                return False

            # Use math.isclose to compare to zero with tolerance
            if not math.isclose(value, 0.0, abs_tol=abs_tol):
                print(f"Error {value} is not close to 0")
                return False

    return True


@then("LOLD.txt and PositiveUnsuppliedEnergy.txt files are full of zeros")
def check_other_outputs(context):
    assert (is_file_full_of_zeros(context.loss_of_load_file))
    assert (is_file_full_of_zeros(context.positive_unsupplied_energy_file))


@then("the expected positive unsupplied energy is")
def check_positive_unsupplied_energy(context):
    results = read_table_from_string(context.positive_unsupplied_energy)
    check_cucumber_table(context, results)


@then("the expected loss of load is")
def check_loss_of_load_is(context):
    results = read_table_from_string(context.lold)
    check_cucumber_table(context, results)


def check_cucumber_table(context, results):
    headers = context.table.headings
    for i, row in enumerate(context.table):
        for header in headers:
            expected_value = float(row[header])
            actual_value = float(results[i][header])

            np.testing.assert_allclose(actual_value, expected_value, rtol=1e-6, atol=0,
                                       err_msg=f"Mismatch in row {i + 1}, column '{header}': expected {expected_value}, got {actual_value}")


def get_results_file_path_from_logs(logs: bytes) -> str:
    for line in logs.splitlines():
        if b'Optimization results available in : ' in line:
            return line.split(b'Optimization results available in : ')[1].decode('ascii')
    raise LookupError("Could not find results file path in output logs")


@then('Simulator has been launched with solver "{string}"')
def check_simulator_solver(context, string):
    string_to_find = f"solver {string} is used for linear problem resolution"
    assert (find_in_simulator_log(context.tmp_study / "output", string_to_find))


@then('Benders has been launched with solver "{string}"')
def check_benders_solver(context, string):
    solver_in_benders = context.options_data["SOLVER_NAME"]
    print(f"Solver in benders: {solver_in_benders}\n")
    assert solver_in_benders.upper() == string.upper()


def run_xpansion_step(context, step, memory_mode, nproc=1):
    """Common function to run an Antares-Xpansion step with error handling"""
    context.allow_run_as_root = get_conf("allow_run_as_root")

    # Parse memory mode
    in_memory = "memory" in memory_mode.lower()

    command = build_launch_command(
        context.tmp_study,
        step=step,
        nproc=nproc,
        in_memory=in_memory,
        allow_run_as_root=context.allow_run_as_root
    )

    print(f"Running {step} {memory_mode}: {' '.join(command)}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    context.return_code = process.returncode

    if context.return_code != 0:
        print(f"{step} failed:")
        print(out.decode('utf-8'))
        print(err.decode('utf-8'))
        return False

    return True


@when(u'I run step {step} {memory_mode} followed by step presolve')
def step_problem_generation_and_presolve(context, step, memory_mode):
    # Run the first step (usually problem_generation)
    if not run_xpansion_step(context, step, memory_mode, nproc=1):
        return

    # Run presolve step
    if not run_xpansion_step(context, "presolve", "on_disk", nproc=1):
        return


@when(u'I run step {step} {memory_mode}')
def step_problem_generation_memory(context, step, memory_mode):
    run_xpansion_step(context, step, memory_mode, nproc=1)


@then(u'the return status is 0')
def check_return_status(context):
    assert context.return_code == 0, f"Expected return code 0, got {context.return_code}"


@then(u'the generated subproblems have between {min} and {max} rows')
def check_subproblems_rows(context, min, max):
    lp_path = context.tmp_study / "lp"
    rows, _, _ = get_subproblem_statistics(lp_path)
    assert int(min) <= rows <= int(max), f"Expected rows between {min}-{max}, got {rows}"


@then(u'the generated subproblems have between {min} and {max} cols')
def check_subproblems_cols(context, min, max):
    lp_path = context.tmp_study / "lp"
    _, cols, _ = get_subproblem_statistics(lp_path)
    assert int(min) <= cols <= int(max), f"Expected cols between {min}-{max}, got {cols}"


@then(u'the generated subproblems have between {min} and {max} elements')
def check_subproblems_elements(context, min, max):
    lp_path = context.tmp_study / "lp"
    _, _, elements = get_subproblem_statistics(lp_path)
    assert int(min) <= elements <= int(max), f"Expected elements between {min}-{max}, got {elements}"


@then(u'the generated subproblems have {n} rows')
def check_subproblems_exact_row(context, n):
    lp_path = context.tmp_study / "lp"
    rows, _, _ = get_subproblem_statistics(lp_path)
    assert rows == int(n), f"Expected exactly {n} rows, got {rows}"


@then(u'the generated subproblems have {n} cols')
def check_subproblems_exact_cols(context, n):
    lp_path = context.tmp_study / "lp"
    _, cols, _ = get_subproblem_statistics(lp_path)
    assert cols == int(n), f"Expected exactly {n} cols, got {cols}"


@then(u'the generated subproblems have {n} elements')
def check_subproblems_exact_elements(context, n):
    lp_path = context.tmp_study / "lp"
    _, _, elements = get_subproblem_statistics(lp_path)
    assert elements == int(n), f"Expected exactly {n} elements, got {elements}"
