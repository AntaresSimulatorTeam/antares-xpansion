import shutil
from pathlib import Path

from antares_xpansion.yearly_weight_writer import YearlyWeightWriter

from file_creation import _create_weight_file, _create_empty_file_from_list


def test_weight_file_write(tmp_path):
    input_weight_file_name = "weight_file"
    file_path: Path = tmp_path / input_weight_file_name
    weight_list = [1.1, 2.1, 3.1]
    active_years = [1, 2, 4]
    _create_weight_file(file_path, weight_list)

    mps_files = (
        f"problem-{active_years[0]}-1-AAAAMMDD-hhmmss.mps", f"problem-{active_years[0]}-24-AAAAMMDD-hhmmss.mps",
        f"problem-{active_years[1]}-1-AAAAMMDD-hhmmss.mps", f"problem-{active_years[1]}-24-AAAAMMDD-hhmmss.mps",
        f"problem-{active_years[2]}-1-AAAAMMDD-hhmmss.mps", f"problem-{active_years[2]}-24-AAAAMMDD-hhmmss.mps")

    _create_empty_file_from_list(tmp_path, mps_files)
    shutil.make_archive(tmp_path, "zip", tmp_path)
    zipped_out = tmp_path.parent / (tmp_path.name + ".zip")
    YearlyWeightWriter(tmp_path, zipped_out).create_weight_file(
        weight_list, input_weight_file_name, active_years)

    expected_content = f"problem-{active_years[0]}-1-AAAAMMDD-hhmmss.mps {weight_list[0]}\n" \
                       f"problem-{active_years[0]}-24-AAAAMMDD-hhmmss.mps {weight_list[0]}\n" \
                       f"problem-{active_years[1]}-1-AAAAMMDD-hhmmss.mps {weight_list[1]}\n" \
                       f"problem-{active_years[1]}-24-AAAAMMDD-hhmmss.mps {weight_list[1]}\n" \
                       f"problem-{active_years[2]}-1-AAAAMMDD-hhmmss.mps {weight_list[2]}\n" \
                       f"problem-{active_years[2]}-24-AAAAMMDD-hhmmss.mps {weight_list[2]}\n" \
                       f"WEIGHT_SUM {sum(weight_list)}"

    with open(YearlyWeightWriter(tmp_path, zipped_out).output_dir / input_weight_file_name, 'r') as write_file:
        content = write_file.read()
        assert content == expected_content
