

import json
from pathlib import Path


class ResumeStudy:
    def __init__(self) -> None:
        pass

    def launch(self, simulation_output_path: Path, options_file: str, last_master_keyword):
        self.set_simulation_output_path(simulation_output_path)
        # last_master_file = simulation_output_path / last_master_file

        self._update_options_file(options_file, last_master_keyword)

    def set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else:
            raise ResumeStudy.StudyOutputPathError(
                f"Study output path error: {simulation_output_path} not found!"
            )

    class StudyOutputPathError(Exception):
        pass

    class LastMasterFileNotFound(Exception):
        pass

    def _update_options_file(self, options_file, last_master_keyword: str):
        options_file_path = self._simulation_output_path / options_file
        if not options_file_path.exists():
            raise ResumeStudy.OptionsFileNotFound(
                f"last master file: {options_file_path} not found")
        with open(self._simulation_output_path / options_file, "r") as json_file:
            options = json.load(json_file)
        try:
            last_master_file = options[last_master_keyword]
        except KeyError:
            raise ResumeStudy.LastMasterKeyWordNotFound(
                f"Error {last_master_keyword} not found in {options_file_path}")
        last_master_file_path = self._simulation_output_path / last_master_file
        if not last_master_file_path.exists():
            raise ResumeStudy.LastMasterFileNotFound(
                f"last master file: {last_master_file_path} not found")

    class OptionsFileNotFound(Exception):
        pass

    class LastMasterKeyWordNotFound(Exception):
        pass
