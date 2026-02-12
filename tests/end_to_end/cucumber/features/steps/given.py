import fileinput
import re
import shutil
from pathlib import Path

from behave import *


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
