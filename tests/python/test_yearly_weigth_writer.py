import os
from pathlib import Path
import touch
import pytest

from src.python.antares_xpansion.input_checker import check_weights_file, InvalidYearsWeightNumber
from src.python.antares_xpansion.xpansion_study_reader import XpansionStudyReader
from src.python.antares_xpansion.yearly_weight_writer import YearlyWeightWriter


def _create_weight_file(file_path, weight_list):
    file_path.write_text(_create_years_content(weight_list))


def _create_years_content(weight_list):
    content = ""
    for weight in weight_list:
        content += f"{weight}\n"

    content += "\n"
    content += "\n"
    return content


def test_check_fails_if_file_does_not_exist(tmp_path):
    file_path: Path = tmp_path / "toto_file"
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 0)
    assert pytest_wrapped_e.type == SystemExit


def test_check_fails_if_there_is_one_negative_value(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    weight_list = [1, -1]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 0)
    assert pytest_wrapped_e.type == SystemExit


def test_check_fails_if_all_values_are_zero(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    weight_list = [0, 0]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 2)
    assert pytest_wrapped_e.type == SystemExit


def test_check_fails_if_nb_activated_years_different_from_content(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    weight_list = [1, 2, 3, 4]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(InvalidYearsWeightNumber):
        check_weights_file(file_path, 5)


def test_xpansion_weight_read(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    weight_list = [1, 2, 3, 4]
    _create_weight_file(file_path, weight_list)
    assert (XpansionStudyReader.get_years_weight_from_file(file_path) == weight_list)


def _create_empty_file(tmp_path: Path, filename):
    file_path = tmp_path / filename
    touch.touch(file_path)


def _create_empty_file_from_list(tmp_path: Path, files):
    for file in files:
        _create_empty_file(tmp_path, file)


def test_weight_file_write(tmp_path):
    input_weight_file_name = "weight_file"
    file_path: Path = tmp_path / input_weight_file_name
    weight_list = [1, 2, 3]
    _create_weight_file(file_path, weight_list)

    mps_files = (
    "problem-1-1-AAAAMMDD-hhmmss.mps", "problem-1-2-AAAAMMDD-hhmmss.mps", "problem-1-24-AAAAMMDD-hhmmss.mps",
    "problem-2-1-AAAAMMDD-hhmmss.mps", "problem-2-2-AAAAMMDD-hhmmss.mps", "problem-2-24-AAAAMMDD-hhmmss.mps",
    "problem-3-1-AAAAMMDD-hhmmss.mps", "problem-3-2-AAAAMMDD-hhmmss.mps", "problem-3-24-AAAAMMDD-hhmmss.mps")

    _create_empty_file_from_list(tmp_path, mps_files)
    YearlyWeightWriter(tmp_path).create_weight_file(weight_list, input_weight_file_name)

    expected_content = "problem-1-1-AAAAMMDD-hhmmss 1\n" \
                       "problem-1-2-AAAAMMDD-hhmmss 1\n" \
                       "problem-1-24-AAAAMMDD-hhmmss 1\n" \
                       "problem-2-1-AAAAMMDD-hhmmss 2\n" \
                       "problem-2-2-AAAAMMDD-hhmmss 2\n" \
                       "problem-2-24-AAAAMMDD-hhmmss 2\n" \
                       "problem-3-1-AAAAMMDD-hhmmss 3\n" \
                       "problem-3-2-AAAAMMDD-hhmmss 3\n" \
                       "problem-3-24-AAAAMMDD-hhmmss 3\n" \
                       "WEIGHT_SUM 6"

    with open(tmp_path / "lp" / input_weight_file_name, 'r') as write_file:
        content = write_file.read()
        assert content == expected_content
