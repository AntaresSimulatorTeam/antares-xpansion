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


def build_exe_command(context, n: int, option_file: str = "options.json",exe_file="OUTER_LOOP"):    
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=n)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf(exe_file)
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
    current_directory = os.getcwd() 
    repo_root = os.path.abspath(os.path.join(current_directory, "../../../../.."))

    excluded_repos = ["docs",".git","tests",".vscode","conception", 
                    "vcpkg","src","examples","cmake",".github","data_test", 
                    "docker"]

    excluded_paths = {os.path.join(repo_root,d) for d in excluded_repos} 
    print(f"repo_root : {repo_root}")

    repos_2_consider = [os.path.join(repo_root,d) for d in os.listdir(repo_root) 
                    if (os.path.isdir(os.path.join(repo_root,d)) and 
                        os.path.join(repo_root,d) not in excluded_paths)]

    for repo_2_consider in repos_2_consider : 
        print(f"repo to consider {repo_2_consider}")
        for item in os.listdir(repo_2_consider) : 
            if item.startswith("benders") : 
                full_item_path = os.path.join(repo_2_consider,item)
                if (os.access(full_item_path,os.X_OK) and os.access(full_item_path,os.X_OK)) : 
                    return full_item_path

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
    command = build_exe_command(context, n, option_file)
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



@when('I run antares-xpansion in trajectory')
def run_trajectory_mode(context):
    """Run the trajectory investment workflow (full step) and load outputs"""
    # Ensure tmp study exists and determine input file
    input_root = Path(context.tmp_study)
    # Default trajectory user input file name for tests
    user_input_file = input_root / 'user_input_XpansionTrajectory.yaml'

    if not user_input_file.exists():
        # Fallback to common names if needed
        for candidate in ['user_input_XpansionTrajectory.yml', 'trajectory.yaml', 'trajectory.yml']:
            cand = input_root / candidate
            if cand.exists():
                user_input_file = cand
                break

    # Build trajectory launch command using the unified launcher with --trajectory flag
    command = [
        sys.executable,
        '../../src/python/launch.py',
        '--trajectory',
        '--installDir', str(get_conf('DEFAULT_INSTALL_DIR')),
        '--input-root', str(input_root),
        '--input-file', str(user_input_file),
        '--step', 'full',
        '--memory'
    ]

    # Allow running as root inside some CI when configured
    if get_conf('allow_run_as_root'):
        command.append('--allow-run-as-root')

    print(f"Running trajectory command: {' '.join(command)}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    context.return_code = process.returncode

    # In case of failure, expose logs for debugging in test output
    if context.return_code != 0:
        print(out.decode('utf-8', errors='ignore'))
        print(err.decode('utf-8', errors='ignore'))
        return

    # On success, read output JSON produced by trajectory resolution
    out_json_path = input_root / 'output' / 'out_benders.json'
    if out_json_path.exists():
        context.outputs = read_json_file(out_json_path)
    else:
        # If output path differs, try to parse from stdout hint if present
        try:
            from .steps import get_results_file_path_from_logs  # noqa: F401
        except Exception:
            get_results_file_path_from_logs = None
        if get_results_file_path_from_logs is not None:
            try:
                inferred = Path(get_results_file_path_from_logs(out))
                if inferred.exists():
                    context.outputs = read_json_file(inferred)
            except Exception:
                pass


@when("I run benders for investment strategy") 
def run_benders_for_investment_strategy(context) :  
    context.allow_run_as_root = get_conf("allow_run_as_root")
    command = build_exe_command(context=context,n=10, exe_file="BENDERS")
    result = subprocess.run(command, capture_output=True, text=True, cwd=context.tmp_study)
    print("printing the result ")
    print(result.stdout)
    print(result.stderr)
    print("end running the mpi command ..... ")


