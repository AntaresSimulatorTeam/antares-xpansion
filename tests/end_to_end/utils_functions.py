import json
import os
import shutil
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
    nproc_str = str(nproc)
    if sys.platform.startswith("win32"):
        return ["mpiexec", "-n", nproc_str]
    elif sys.platform.startswith("linux"):
        MPI_LAUNCHER = "mpirun"
        MPI_N = "-np"
        if allow_run_as_root:
            return [MPI_LAUNCHER, "--allow-run-as-root", MPI_N, nproc_str, "--oversubscribe"]
        else:
            return [MPI_LAUNCHER, MPI_N, nproc_str, "--oversubscribe"]


def remove_outputs(study_path):
    output_path = study_path / "output"
    if os.path.isdir(output_path):
        shutil.rmtree(output_path)
    os.makedirs(output_path)

def get_filepath(output_dir, folder, filename):
    op = []
    for path in Path(output_dir).iterdir():
        for jsonpath in Path(path / folder).rglob(filename):
            op.append(jsonpath)
    assert len(op) == 1
    return op[0]


def read_file(output_path):
    with open(output_path, 'r') as file:
        outputs = file.readlines()
    return outputs


class FilesToRead:
    out_json: Path
    options_json: Path
    lold: Path = None
    positive_unsupplied_energy: Path = None


class Outputs:
    out_json: str
    options_json: str
    lold: str
    positive_unsupplied_energy: str


def get_out_data(output_dir, files_to_read: FilesToRead) -> Outputs:
    for path in Path(output_dir).iterdir():
        if path.suffix == ".zip":
            with zipfile.ZipFile(path, "r") as archive:
                out = Outputs()
                out.out_json = json.loads(archive.read(files_to_read.out_json.as_posix()))
                out.options_json = json.loads(archive.read(files_to_read.options_json.as_posix()))
                if files_to_read.lold:
                    out.lold = archive.read(files_to_read.lold.as_posix()).decode('utf-8')
                if files_to_read.positive_unsupplied_energy:
                    out.positive_unsupplied_energy = archive.read(
                        files_to_read.positive_unsupplied_energy.as_posix()).decode(
                        'utf-8')

                return out
    return None


def read_outputs(output_path, use_archive=True, lold=False, positive_unsupplied_energy=False):
    files_to_read = FilesToRead()
    files_to_read.out_json = Path("expansion") / "out.json"
    files_to_read.options_json = Path("lp") / "options.json"

    if lold:
        files_to_read.lold = Path("lp") / "LOLD.txt"
    if positive_unsupplied_energy:
        files_to_read.positive_unsupplied_energy = Path("lp") / "PositiveUnsuppliedEnergy.txt"

    if use_archive:
        return get_out_data(output_path, files_to_read)
    else:
        out = Outputs()
        json_path = get_filepath(output_path, "expansion", "out.json")
        options_path = get_filepath(output_path, "lp", "options.json")

        with open(str(json_path), "r") as json_file:
            out.out_json = json.load(json_file)

        with open(str(options_path), "r") as options_file:
            out.options_json = json.load(options_file)

        if lold:
            out.lold = read_file(get_filepath(output_path, "lp", "LOLD.txt"))
        if positive_unsupplied_energy:
            out.positive_unsupplied_energy = read_file(
                get_filepath(output_path, "lp", "PositiveUnsuppliedEnergy.txt"))

        return out
