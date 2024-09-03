import os
import subprocess
from pathlib import Path

from behave import *

from utils_functions import get_mpi_command, get_conf


@given('the study path is "{string}"')
def study_path_is(context, string):
    context.options_file = os.path.join(Path() / "../../",
                                        string.replace("/", os.sep), "lp", "options.json")
    print(f"********** context.options_file={context.options_file} ")


def build_outer_loop_command(context):
    command = get_mpi_command(allow_run_as_root=context.allow_run_as_root, nproc=context.nproc)
    exe_path = Path(get_conf("DEFAULT_INSTALL_DIR")) / get_conf("OUTER_LOOP")
    command.append(str(exe_path))
    command.append(context.options_file)
    return command


# outer_loop_executable = outer_loop_executable()


@when('I run outer loop with {n} proc(s)')
def run_outer_loop(context, n):
    context.nproc = int(n)
    context.allow_run_as_root = get_conf("allow_run_as_root")
    command = build_outer_loop_command(context)
    print(f"Running command: {command}")
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    out, err = process.communicate()
    print(out)
    print("*****")
    print(err)
    # context.output_path = parse_output_folder_from_logs(out)
    # context.return_code = process.returncode
