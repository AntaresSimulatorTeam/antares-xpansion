from pathlib import Path

import pytest

from src.python.antares_xpansion.input_checker import check_weights_file


def test_check_fails_if_file_does_not_exist(tmp_path):
    file_path: Path = tmp_path / "toto_file"
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 0)
    assert pytest_wrapped_e.type == SystemExit


def test_check_fails_if_there_is_one_negative_value(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    content = "1\n" \
              "-1"
    file_path.write_text(content)
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 0)
    assert pytest_wrapped_e.type == SystemExit


def test_check_fails_if_all_values_are_zero(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    content = "0\n" \
              "0"
    file_path.write_text(content)
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        check_weights_file(file_path, 0)
    assert pytest_wrapped_e.type == SystemExit
