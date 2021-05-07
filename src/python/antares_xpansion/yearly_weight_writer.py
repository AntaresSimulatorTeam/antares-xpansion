import os
from pathlib import Path
from typing import List


class YearlyWeightWriter:

    def __init__(self, simulation_path: Path):
        self.MPS_DIR = "lp"
        self.simulation_path = simulation_path
        self.file_content = []
        if not os.path.isdir(self.output_dir):
            os.mkdir(self.output_dir)

    @property
    def output_dir(self):
        return self.simulation_path / self.MPS_DIR

    def create_weight_file(self, weight_list: List[float], file_name: str):
        self.file_content = []
        self._add_mps_lines_with_weights_to_content(weight_list)
        self._add_last_line_to_content(weight_list)
        self._write_content_to_file(file_name)

    def _add_mps_lines_with_weights_to_content(self, weight_list):
        sorted_dir = sorted(os.listdir(self.simulation_path))
        for instance in sorted_dir:
            if self._file_should_be_added(instance):
                self._add_line_to_file_content(instance, weight_list)

    def _add_line_to_file_content(self, file_name, weight_list):
        year = self._get_year_index_from_name(file_name)
        mps_file_name = Path(file_name).with_suffix('').name
        self.file_content.append(mps_file_name + " " + str(weight_list[year - 1]) + "\n")

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
        return '.mps' in file_name and not '-1.mps' in file_name
