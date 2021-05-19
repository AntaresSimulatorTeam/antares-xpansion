import yaml
import os
from pathlib import Path


def get_install_dir():
    with open(Path(os.path.abspath(__file__)).parent / 'build_config.yaml') as file:
        content = yaml.full_load(file)
        if content is not None:
            return content.get('installDir', "cmake-build-release")
        else:
            raise RuntimeError("Please check file config.yaml, content is empty")
