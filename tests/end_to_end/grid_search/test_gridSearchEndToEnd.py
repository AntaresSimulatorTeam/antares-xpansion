import csv
import pytest
import json
import os
import shutil
import subprocess
from pathlib import Path
import numpy as np
from os import listdir
import zipfile
import pdb


from tests.end_to_end.utils_functions import get_conf, get_mpi_command

## TESTS ##


@pytest.mark.optim
def test_001_grid_search(install_dir, allow_run_as_root, tmp_path, xpress):
    run_solver(install_dir, "BENDERS", tmp_path, allow_run_as_root, False, xpress)


# File STUDIES_FILE_PATH
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
STUDIES_FILE_PATH = Path("studies.json")


# launch_optimization : launches optimization part of AntaresXpansion
# arguments :
#   - data_path   : Folder containing instance MPS, structure and option files
#                   Executable will be launched from this folder


def launch_problem_generation(data_path):

    script_path = str(
        (
            (Path(__file__).parent.parent.parent.parent) / "src/python/launch.py"
        ).resolve()
    )

    # Arguments to pass
    arguments = ["-i", data_path, "--step", "antares"]
    # Call the script using subprocess
    subprocess.run(["python3", script_path] + arguments, capture_output=True, text=True)

    arguments = ["-i", data_path, "--step", "problem_generation"]
    subprocess.run(["python3", script_path] + arguments, capture_output=True, text=True)

    # Deleted the zip
    # filter list_dir to get the zip file (end with .zip)
    zip_file = list(
        filter(lambda x: x.endswith(".zip"), listdir(data_path / "output"))
    )[0]
    zip_path = data_path / "output" / zip_file
    os.remove(zip_path)

    mps_path = data_path / "output" / listdir(data_path / "output")[0]
    # Copy grid.csv from data_path/user/expansion/grid.csv to mps_path
    shutil.copy(data_path / "user/expansion/grid.csv", mps_path / "lp")

    return mps_path


# Used to generate the grid.csv files from the out.json files
# def export_grid_points_to_gridcsv(benders_points_refs):
#     with open(benders_points_refs, "r") as jsonFile:
#         curr_instance_json = json.load(jsonFile)

#     iterations = curr_instance_json["iterations"]
#     grid_points = {}

#     for i in iterations:
#         # Create an inner dictionary for each iteration
#         grid_points[str(i)] = {}

#         # Populate the inner dictionary for this iteration
#         for candidate in curr_instance_json["iterations"][i]["candidates"]:
#             grid_points[str(i)][candidate["name"]] = candidate["invest"]

#     # Write the grid points to a CSV file with header using the candidate names
#     with open(
#         benders_points_refs.parent.parent / "expansion/grid.csv", "w", newline=""
#     ) as csvfile:
#         writer = csv.writer(csvfile)
#         # Write the header row
#         writer.writerow([""] + list(grid_points["1"].keys()))

#         # Write the data rows
#         for iteration in grid_points:
#             writer.writerow([iteration] + list(grid_points[iteration].values()))


def launch_grid_search(data_path, executable):
    # Going to instance folder
    owd = os.getcwd()
    os.chdir(data_path)

    command = [executable, data_path]

    # Launching optimization from instance folder
    subprocess.run(command, stdout=subprocess.PIPE, stderr=None)

    os.chdir(owd)


# check_optimization_json_output : Compares values written by optimization in file
#                 'out.jons' with expected values, written in resultTest.json.
# arguments :
#   - expected_results_dict   : Dict of expected values to compare with ones present in
#                               in file out.json
#   - output_path             : Path to the output file out.json


def check_gridpoints_values(benders_points_refs: Path, gridsearch_points_values: Path):

    # Loading output from optimization process
    curr_instance_json = {}

    with open(benders_points_refs, "r") as jsonFile:
        curr_instance_json = json.load(jsonFile)

    with open(gridsearch_points_values, "r") as jsonFile:
        gridsearch_points_values = json.load(jsonFile)

    # Check if the number of iterations is the same
    assert len(curr_instance_json["iterations"]) == len(
        gridsearch_points_values["grid_points"]
    ), f"Number of iterations mismatch: {len(curr_instance_json['iterations'])} vs {len(gridsearch_points_values['grid_points'])}"

    # Check if each point from benders iterations has the same value in gridsearch
    for iteration in curr_instance_json["iterations"]:
        benders_it = curr_instance_json["iterations"][iteration]
        gridsearch_it = gridsearch_points_values["grid_points"][iteration]

        assert (
            abs(benders_it["operational_cost"] - gridsearch_it["operational_cost"])
            < 1e-6
        ), f"Operational cost mismatch: {benders_it['operational_cost']} vs {gridsearch_it['operational_cost']}"

        assert (
            abs(benders_it["investment_cost"] - gridsearch_it["investment_cost"]) < 1e-6
        ), f"Investment cost mismatch: {benders_it['investment_cost']} vs {gridsearch_it['investment_cost']}"

        assert (
            abs(benders_it["overall_cost"] - gridsearch_it["overall_cost"]) < 1e-6
        ), f"Overall cost mismatch: {benders_it['overall_cost']} vs {gridsearch_it['overall_cost']}"


def run_solver(
    install_dir, solver, tmp_path, allow_run_as_root=False, mpi=False, xpress=False
):
    # Loading expected results from json STUDIES_FILE_PATH
    with open(STUDIES_FILE_PATH, "r") as jsonFile:
        scenarios = json.load(jsonFile)

    solver_executable = get_conf("BENDERS")
    pre_command = []

    if mpi:
        pre_command = get_mpi_command(allow_run_as_root, 2)

    benders_executable_path = str(
        (Path(install_dir) / Path(solver_executable)).resolve()
    )
    grid_search_executable_path = str(
        (Path(install_dir) / Path(get_conf("GRID_SEARCH"))).resolve()
    )

    for instance in scenarios:
        # Print the name of the instance being tested
        print(f"Testing scenario: {instance}")
        instance_path = Path(scenarios[instance]["path"])
        command = [e for e in pre_command]
        command.append(benders_executable_path)

        tmp_study = tmp_path / (instance_path.name)
        shutil.copytree(instance_path, tmp_study)

        # launch_benders_optimization(tmp_study, command)
        # Call src/python/launch.py using arguments
        mps_path = launch_problem_generation(tmp_study)

        launch_grid_search(mps_path / "lp", grid_search_executable_path)

        check_gridpoints_values(tmp_study / "user/expansion/out.json", mps_path / "lp/output.json")
