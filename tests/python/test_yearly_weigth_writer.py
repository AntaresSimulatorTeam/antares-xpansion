from pathlib import Path

from src.python.antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from tests.python.file_creation import _create_weight_file, _create_empty_file_from_list


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
