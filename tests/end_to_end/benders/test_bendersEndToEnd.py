import json
import os
import shutil
import subprocess
from pathlib import Path
import yaml
import numpy as np
import pytest

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
RESULT_FILE_PATH = Path('resultTest.json')

# File CONFIG_FILE_PATH
# yaml file containing executable name
CONFIG_FILE_PATH = Path.cwd() / ".."/ ".." / ".."/ "src" / 'python' / 'config.yaml'

def remove_outputs(study_path):
    output_path = study_path / 'output'
    for f in Path(output_path).iterdir():
        if f.is_dir():
            shutil.rmtree(f)

# launch_optimization : launches optimization part of AntaresXpansion
# arguments :
#   - data_path   : Folder containing instance MPS, structure and option files
#                   Executable will be launched from this folder
#   - commands    : list of commands to launch
#                   Example : ['python3', 'test.py', '--arg', 'argValue']
#   - status      : problem status of current instance
#                   If this status is not 'OPTIMAL', then the process might return a 
#                   code different from 0.
def launch_optimization(data_path, commands, status):

    # Going to instance folder
    owd = os.getcwd()
    os.chdir(data_path)

    # Launching optimization from instance folder
    process = subprocess.run(commands, stdout=subprocess.PIPE, stderr=None)

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
#   - instance_path           : Path to intance folder, to find output file 'out.json'
def check_optimization_json_output(expected_results_dict, instance_path) :

    # Loading output from optimization process
    curr_instance_json = {}

    output_path = expected_results_dict['path'] + "/outputs/out.json"
    with open(output_path, 'r') as jsonFile:
        curr_instance_json = json.load(jsonFile)

    # Testing optimality
    prb_status = curr_instance_json['solution']['problem_status']
    assert prb_status == expected_results_dict['status']

    # Testing optimization output if the instance has a solution
    if(prb_status == 'OPTIMAL') :
        value = curr_instance_json['solution']['overall_cost']
        optimal_variables = np.array([i[1] for i in \
            sorted(curr_instance_json['solution']['values'].items())])
        

        #Testing optimal value
        np.testing.assert_allclose(value, \
            expected_results_dict['optimal_value'], rtol=1e-6, atol=0)

        #Testing optmal solution (variables)
        # Sorting the Dict tuples (key, val) by alphabetic order then taking the values
        # for comparison
        expected_sorted_values = np.array([i[1] for i in \
            sorted(expected_results_dict['optimal_variables'].items())])
        np.testing.assert_allclose(optimal_variables, expected_sorted_values, \
                rtol=1e-6, atol=0)


def run_solver(installDir, solver : str):
    # Loading expected results from json RESULT_FILE_PATH
    with open(RESULT_FILE_PATH, 'r') as jsonFile:
        expected_results_dict = json.load(jsonFile)

    with open(CONFIG_FILE_PATH) as file:
        content = yaml.full_load(file)
        if content is not None:
            solver_executable = content.get(solver, solver)
        else:
            raise RuntimeError("Please check file config.yaml, content is empty")

    for instance in expected_results_dict:
        instance_path = expected_results_dict[instance]['path']

        executable_path = str((Path(installDir) / Path(solver_executable)).resolve())
        commands = [executable_path,
                    expected_results_dict[instance]['option_file']
                    ]

        launch_optimization(instance_path, commands, expected_results_dict[instance]["status"])
        check_optimization_json_output(expected_results_dict[instance], instance_path)

## TESTS ##
@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(installDir):
    run_solver(installDir, 'BENDERS_SEQUENTIAL')


@pytest.mark.optim
@pytest.mark.mergemps
def test_001_mergemps(installDir):
    run_solver(installDir, 'MERGE_MPS')


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(installDir):
    run_solver(installDir, 'BENDERS_MPI')
