import json
import os
import subprocess
import sys
from pathlib import Path

from behave import *
from utils_functions import get_conf, remove_outputs, read_outputs, get_mpi_command


def read_json_file(output_path):
    with open(output_path, 'r') as file:
        outputs = json.load(file)
    return outputs


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


def build_outer_loop_command(context, n: int, option_file: str = "options.json"):
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=n)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf("OUTER_LOOP")
    command.append(str(exe_path))
    command.append(option_file)
    return command


def build_launch_command(study_dir: Path, step: str = None, nproc: int = 2, in_memory: bool = False,
                         method: str = None, allow_run_as_root: bool = False, problem_format: str = None):
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
    if problem_format is not None:
        command.extend(["--problem-format", problem_format])
    return command


def run_xpansion_step(context, step, memory_mode, pb_format=None, nproc=1):
    """Common function to run an Antares-Xpansion step with error handling"""
    context.allow_run_as_root = get_conf("allow_run_as_root")

    # Parse memory mode
    in_memory = "memory" in memory_mode.lower()

    command = build_launch_command(
        context.tmp_study,
        step=step,
        nproc=nproc,
        in_memory=in_memory,
        allow_run_as_root=context.allow_run_as_root,
        problem_format=pb_format
    )

    print(f"Running {step} {memory_mode} {pb_format}: {' '.join(command)}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    context.return_code = process.returncode

    if context.return_code != 0:
        print(f"{step} failed:")
        print(out.decode('utf-8'))
        print(err.decode('utf-8'))
        return False

    return True


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


@when(u'I run step {step} {memory_mode} followed by step presolve')
def step_problem_generation_and_presolve(context, step, memory_mode):
    # Run the first step (usually problem_generation)
    if not run_xpansion_step(context, step, memory_mode, pb_format='mps', nproc=1):
        return

    # Run presolve step
    if not run_xpansion_step(context, "presolve", "on_disk", pb_format='mps', nproc=1):
        return


@when(u'I run step {step}')
@when(u'I run step {step} {memory_mode}')
@when(u'I run step {step} {memory_mode} {pb_format}')
def step_problem_generation_memory(context, step, memory_mode=None, pb_format=None):
    run_xpansion_step(context, step, memory_mode, pb_format, nproc=1)
