import os
import sys
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
