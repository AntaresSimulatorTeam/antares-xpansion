import fileinput
import json
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
    with fileinput.FileInput(
        str(context.tmp_study / "user" / "expansion" / "settings.ini"), inplace=True
    ) as file:
        for line in file:
            match = match or re.search(r"solver\s*=.*", line)
            print(re.sub(r"solver\s*=.*", f"solver= {string}", line), end="")
    if not match:
        with open(context.tmp_study / "user/expansion/settings.ini", "a") as file:
            file.write(f"\nsolver = {string}")


@given("the batch size is {batch_size}")
def set_batch_size(context, batch_size):
    context.batch_size = batch_size
    with open(str(context.tmp_study / "options.json"), "r") as file:
        options_content = json.load(file)
    options_content["BATCH_SIZE"] = int(batch_size)
    with open(str(context.tmp_study / "options.json"), "w") as file:
        json.dump(options_content, file, indent=4)



    
@given("the cache problems level is {level}")
def set_cache_problems_level(context, level):
    with open(str(context.tmp_study / "options.json"), "r") as file:
        options_content = json.load(file)
    options_content["CACHE_PROBLEMS"] = int(level)
    with open(str(context.tmp_study / "options.json"), "w") as file:
        json.dump(options_content, file, indent=4)


