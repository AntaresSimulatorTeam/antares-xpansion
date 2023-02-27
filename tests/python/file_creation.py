import os.path
from pathlib import Path


def _create_weight_file(file_path, weight_list):
    file_path.write_text(_create_years_content(weight_list))


def _create_years_content(weight_list):
    content = ""
    for weight in weight_list:
        content += f"{weight}\n"

    content += "\n"
    content += "\n"
    return content


def _create_empty_file(tmp_path: Path, filename):
    empty_file = os.path.join(tmp_path, filename)
    Path(empty_file).touch()


def _create_empty_file_from_list(tmp_path: Path, files):
    for file in files:
        _create_empty_file(tmp_path, file)