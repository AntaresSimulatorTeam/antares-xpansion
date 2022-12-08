import os
from pathlib import Path

import yaml


def get_install_dir():
    with open(Path(os.path.abspath(__file__)).parent / 'build_config.yaml') as file:
        content = yaml.full_load(file)
        if content is not None:
            return content.get('installDir', "cmake-build-release")
        else:
            raise RuntimeError("Please check file build_config.yaml, content is empty")

def get_antares_solver():
    with open(Path(os.path.abspath(__file__)).parent / 'build_config.yaml') as file:
        content = yaml.full_load(file)
        if content is not None:
            return content.get('antares_solver', "")
        else:
            raise RuntimeError("Please check file build_config.yaml, content is empty")

def get_antares_solver_path() -> Path:
    return Path(get_install_dir()) / get_antares_solver()

def get_lp_namer_exe():
    with open(Path(os.path.abspath(__file__)).parent / 'build_config.yaml') as file:
        content = yaml.full_load(file)
        if content is not None:
            return content.get('lp_namer', "")
        else:
            raise RuntimeError("Please check file build_config.yaml, content is empty")

def get_lp_namer_path():
    return Path(get_install_dir()) / get_lp_namer_exe()