from pathlib import Path

import pytest

from file_creation import _create_weight_file

from antares_xpansion.xpansion_study_reader import XpansionStudyReader


def test_check_fails_if_file_does_not_exist(tmp_path):
    file_path: Path = tmp_path / "toto_file"
    expected_message = f'Illegal value : {str(file_path)} is not an existent yearly-weights file'
    with pytest.raises(FileNotFoundError) as expect:
        XpansionStudyReader.check_weights_file(file_path, 0)
    assert str(expect.value) == expected_message


def test_check_fails_if_there_is_one_negative_value(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    expected_message = f'Line 2 in file {str(file_path)} indicates a negative value'
    weight_list = [1, -1]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(XpansionStudyReader.InvalidYearsWeightValue) as expect:
        XpansionStudyReader.check_weights_file(file_path, 0)
    assert str(expect.value) == expected_message


def test_check_fails_if_all_values_are_zero(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    expected_message = f'file {str(file_path)} : all values are null'
    weight_list = [0, 0]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(XpansionStudyReader.OnlyNullYearsWeightValue) as expect:
        XpansionStudyReader.check_weights_file(file_path, 2)
    assert str(expect.value) == expected_message


def test_check_fails_if_nb_activated_years_different_from_content(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    expected_message = f'file {str(file_path)} : invalid weight number : 4 values / 5 expected'
    weight_list = [1, 2, 3, 4]
    _create_weight_file(file_path, weight_list)
    with pytest.raises(XpansionStudyReader.InvalidYearsWeightNumber) as expect:
        XpansionStudyReader.check_weights_file(file_path, 5)
    assert str(expect.value) == expected_message


def test_xpansion_weight_read(tmp_path):
    file_path: Path = tmp_path / "weight_file"
    weight_list = [1, 2, 3, 4]
    _create_weight_file(file_path, weight_list)
    assert (XpansionStudyReader.get_years_weight_from_file(file_path) == weight_list)
