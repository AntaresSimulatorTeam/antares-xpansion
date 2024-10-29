import json
import os
import sys
import zipfile
from pathlib import Path

import yaml

# File CONFIG_FILE_PATH
# yaml file containing executable name
CONFIG_FILE_PATH = Path(os.path.abspath(__file__)).parent / ".." / ".." / \
                   "src" / 'python' / 'config.yaml'


def get_conf(key: str):
    solver_executable = ""
    with open(CONFIG_FILE_PATH) as file:
        content = yaml.full_load(file)
        if content is not None:
            solver_executable = content.get(key)
        else:
            raise RuntimeError(
                "Please check file config.yaml, content is empty")

    return solver_executable


def get_mpi_command(allow_run_as_root=False, nproc: int = 1):
    MPI_LAUNCHER = ""
    MPI_N = ""
    nproc_str = str(nproc)
    if sys.platform.startswith("win32"):
        MPI_LAUNCHER = "mpiexec"
        MPI_N = "-n"
        return [MPI_LAUNCHER, MPI_N, nproc_str]
    elif sys.platform.startswith("linux"):
        MPI_LAUNCHER = "mpirun"
        MPI_N = "-np"
        if allow_run_as_root:
            return [MPI_LAUNCHER, "--allow-run-as-root", MPI_N, nproc_str, "--oversubscribe"]
        else:
            return [MPI_LAUNCHER, MPI_N, nproc_str, "--oversubscribe"]


def get_json_filepath(output_dir, folder, filename):
    op = []
    for path in Path(output_dir).iterdir():
        for jsonpath in Path(path / folder).rglob(filename):
            op.append(jsonpath)
    assert len(op) == 1
    return op[0]


def get_json_file_data(output_dir, folder, filename):
    data = None
    for path in Path(output_dir).iterdir():
        if path.suffix == ".zip":
            with zipfile.ZipFile(path, "r") as archive:
                data = json.loads(archive.read(folder + "/" + filename))
    return data


def read_outputs(output_path, use_archive):
    if use_archive:
        json_data = get_json_file_data(output_path, "expansion", "out.json")
        options_data = get_json_file_data(output_path, "lp", "options.json")
    else:
        json_path = get_json_filepath(output_path, "expansion", "out.json")
        options_path = get_json_filepath(output_path, "lp", "options.json")

        with open(str(json_path), "r") as json_file:
            json_data = json.load(json_file)

        with open(str(options_path), "r") as options_file:
            options_data = json.load(options_file)
    return json_data, options_data
