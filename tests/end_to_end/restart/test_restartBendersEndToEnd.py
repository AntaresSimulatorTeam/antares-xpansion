import json
import os
import shutil
import subprocess
from pathlib import Path
import sys
from zipfile import ZIP_DEFLATED, ZipFile
import yaml
import numpy as np

# File RESULT_FILE_PATH
# Json file containing
#   . One entry by instance to test
#       - path : a path to the instance
#       - status : a status of instance resulting of optimization process
#           possible values : OPTIMAL, ERROR
#       - optimal_value : an objective value at the end of optimization process
#           if one exists (NULL otherwise)
#       - optimal_variables :
#           * one entry by variable with variable name as key : optimal
#               value of the variable

# File CONFIG_FILE_PATH
# yaml file containing executable name
CONFIG_FILE_PATH = Path.cwd() / ".." / ".." / ".." / \
    "src" / 'python' / 'config.yaml'


def remove_outputs(study_path):
    output_path = study_path / 'output'
    expansion_dir = study_path / "expansion"
    for f in Path(output_path).iterdir():
        if f.is_dir():
            shutil.rmtree(f)

    if expansion_dir.is_dir():
        shutil.rmtree(expansion_dir)

# launch_optimization : launches optimization part of AntaresXpansion
# arguments :
#   - data_path   : Folder containing instance MPS, structure and option files
#                   Executable will be launched from this folder
#   - commands    : list of commands to launch
#                   Example : ['python3', 'test.py', '--arg', 'argValue']
#   - status      : problem status of current instance
#                   If this status is not 'OPTIMAL', then the process might return a
#                   code different from 0.


def launch_optimization(data_path, commands, status=None):

    # Going to instance folder
    owd = os.getcwd()
    data_path_ = Path(data_path)
    expansion_dir = data_path_ / "expansion"
    assert expansion_dir.is_dir()
    os.chdir(data_path)

    # Launching optimization from instance folder
    process = subprocess.run(
        commands, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Back to working directory
    os.chdir(owd)

    # Check return value
    if status == 'OPTIMAL':
        assert process.returncode == 0

# check_optimization_json_output : Compares values written by optimization in file
#                 'out.jons' with expected values, written in resultTest.json.
# arguments :
#   - expected_results_dict   : Dict of expected values to compare with ones present in
#                               in file out.json
#   - output_path             : Path to the output file out.json


def check_optimization_json_output(expected_results_dict, output_path: Path):

    # Loading output from optimization process
    curr_instance_json = {}

    # output_path = expected_results_dict['path'] / "expansion/out.json"
    with open(output_path, 'r') as jsonFile:
        curr_instance_json = json.load(jsonFile)

    prb_status = None
    # Testing optimality
    if ('solution' in curr_instance_json):
        prb_status = curr_instance_json['solution']['problem_status']
        assert prb_status == expected_results_dict['status']

    # Testing optimization output if the instance has a solution
    if(prb_status == 'OPTIMAL'):
        value = curr_instance_json['solution']['overall_cost']
        optimal_variables = np.array([i[1] for i in
                                      sorted(curr_instance_json['solution']['values'].items())])

        # Testing optimal value
        np.testing.assert_allclose(value,
                                   expected_results_dict['optimal_value'], rtol=1e-6, atol=0)

        # Testing optmal solution (variables)
        # Sorting the Dict tuples (key, val) by alphabetic order then taking the values
        # for comparison
        expected_sorted_values = np.array([i[1] for i in
                                           sorted(expected_results_dict['optimal_variables'].items())])
        np.testing.assert_allclose(optimal_variables, expected_sorted_values,
                                   rtol=1e-6, atol=0)


def run_solver(install_dir, solver, tmp_study, instance, allow_run_as_root=False):

    solver_executable = get_solver_exe(solver)

    pre_command = []

    if (solver == "BENDERS_MPI"):
        pre_command = get_mpi_command(allow_run_as_root)

    executable_path = str(
        (Path(install_dir) / Path(solver_executable)).resolve())

    command = [e for e in pre_command]
    command.append(executable_path)
    command.append(
        instance['option_file']
    )
    status = instance["status"] if "status" in instance else None

    expansion_dir = tmp_study / "expansion"
    if expansion_dir.is_dir():
        shutil.rmtree(expansion_dir)

    expansion_dir.mkdir()
    shutil.copyfile(tmp_study / instance["output_file"],
                    expansion_dir / "out.json")
    shutil.copyfile(tmp_study / instance["last_iteration_file"],
                    expansion_dir / "last_iteration.json")

    launch_optimization(tmp_study, command, status)
    check_optimization_json_output(
        instance, tmp_study/"expansion/out.json")


def get_solver_exe(solver: str):
    solver_executable = ""
    with open(CONFIG_FILE_PATH) as file:
        content = yaml.full_load(file)
        if content is not None:
            solver_executable = content.get(solver, solver)
        else:
            raise RuntimeError(
                "Please check file config.yaml, content is empty")

    return solver_executable


def get_mpi_command(allow_run_as_root=False):
    MPI_LAUNCHER = ""
    MPI_N = ""
    nproc = "2"
    if sys.platform.startswith("win32"):
        MPI_LAUNCHER = "mpiexec"
        MPI_N = "-n"
        return [MPI_LAUNCHER, MPI_N, nproc]
    elif sys.platform.startswith("linux"):
        MPI_LAUNCHER = "mpirun"
        MPI_N = "-np"
        if (allow_run_as_root):
            return [MPI_LAUNCHER, "--allow-run-as-root", MPI_N, nproc, "--oversubscribe"]
        else:
            return [MPI_LAUNCHER, MPI_N, nproc, "--oversubscribe"]
