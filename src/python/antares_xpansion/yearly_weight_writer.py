import os
from pathlib import Path
from typing import List


def strip_end(text, suffix):
    if suffix and text.endswith(suffix):
        return text[:-len(suffix)]
    return text


class YearlyWeightWriter:

    def __init__(self, simulation_path: Path):
        self.MPS_DIR = "lp"
        self.simulation_path = simulation_path
        if not os.path.isdir(self.output_dir):
            os.mkdir(self.output_dir)

    @property
    def output_dir(self):
        return self.simulation_path / self.MPS_DIR

    def create_weight_file(self, weight_list: List[float], file_name: str):
        sorted_dir = sorted(os.listdir(self.simulation_path))
        content = []
        for instance in sorted_dir:
            if '.mps' in instance and not '-1.mps' in instance:
                buffer_l = instance.strip().split("-")
                year = int(buffer_l[1])
                mps_file_name = strip_end(instance, ".mps")
                content.append(mps_file_name + " " + str(weight_list[year - 1]) + "\n")
        content.append("WEIGHT_SUM " + str(sum(weight_list)))
        with open(self.output_dir / file_name, 'w') as weight_file:
            weight_file.writelines(content)
