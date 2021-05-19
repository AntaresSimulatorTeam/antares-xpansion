from pathlib import Path

import touch


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
    file_path = tmp_path / filename
    touch.touch(file_path)


def _create_empty_file_from_list(tmp_path: Path, files):
    for file in files:
        _create_empty_file(tmp_path, file)