import os
from pathlib import Path
from typing import List
from zipfile import ZipFile


class YearlyWeightWriter:

    def __init__(self, xpansion_output_dir: Path, zipped_output_path: Path):
        self.MPS_DIR = "lp"
        self.zipped_output_path = zipped_output_path
        self.xpansion_output_dir = xpansion_output_dir
        self.file_content = []
        if not os.path.isdir(self.output_dir):
            os.mkdir(self.output_dir)

    @property
    def output_dir(self):
        return self.xpansion_output_dir / self.MPS_DIR

    def create_weight_file(self, weight_list: List[float], file_name: str, active_years):
        self.file_content = []
        self._add_mps_lines_with_weights_to_content(weight_list, active_years)
        self._add_last_line_to_content(weight_list)
        self._write_content_to_file(file_name)

    def _add_mps_lines_with_weights_to_content(self, weight_list, active_years):
        with ZipFile(self.zipped_output_path, 'r') as zip_object:
            sorted_dir = sorted(zip_object.namelist())
        for mps_file in sorted_dir:
            if self._file_should_be_added(mps_file):
                self._add_mps_file_to_output_file_content(
                    mps_file, weight_list,  active_years)

    def _add_mps_file_to_output_file_content(self, file_name, weight_list,  active_years):
        year = self._get_year_index_from_name(file_name)
        year_index = active_years.index(int(year))
        mps_file_name = Path(file_name).name
        self.file_content.append(
            mps_file_name + " " + str(weight_list[year_index]) + "\n")

    def _add_last_line_to_content(self, weight_list):
        self.file_content.append("WEIGHT_SUM " + str(sum(weight_list)))

    def _write_content_to_file(self, file_name):
        with open(self.output_dir / file_name, 'w') as weight_file:
            weight_file.writelines(self.file_content)

    @staticmethod
    def _get_year_index_from_name(file_name):
        buffer_l = file_name.strip().split("-")
        year = int(buffer_l[1])
        return year

    @staticmethod
    def _file_should_be_added(file_name) -> bool:
        return '.mps' in file_name
