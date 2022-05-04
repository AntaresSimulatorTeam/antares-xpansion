

import json
from pathlib import Path
import shutil


class ResumeStudy:
    def __init__(self) -> None:
        pass

    def launch(self, simulation_output_path: Path, options_file: str, last_master_keyword, master_keyword):
        self.set_simulation_output_path(simulation_output_path)
        # last_master_file = simulation_output_path / last_master_file

        self._update_options_file(
            options_file, last_master_keyword, master_keyword)

    def set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else:
            raise ResumeStudy.StudyOutputPathError(
                f"Study output path error: {simulation_output_path} not found!"
            )

    class StudyOutputPathError(Exception):
        pass

    def _update_options_file(self, options_file, last_master_keyword: str, master_keyword: str):
        options_file_path = self._simulation_output_path / options_file
        if not options_file_path.exists():
            raise ResumeStudy.OptionsFileNotFound(
                f"last master file: {options_file_path} not found")
        options_file_path = self._simulation_output_path / options_file
        with open(options_file_path, "r") as json_file:
            options = json.load(json_file)
        try:
            last_master_file = options[last_master_keyword]
        except KeyError:
            raise ResumeStudy.KeyWordNotFound(
                f"Error {last_master_keyword} not found in {options_file_path}")
        last_master_file_path = self._simulation_output_path / last_master_file

        if not last_master_file_path.exists():
            raise ResumeStudy.LastMasterFileNotFound(
                f"last master file: {last_master_file_path} not found")

        try:
            master_file = options[master_keyword]
        except KeyError:
            raise ResumeStudy.KeyWordNotFound(
                f"Error {master_keyword} not found in {options_file_path}")
        master_file_path = self._simulation_output_path / master_file
        if not master_file_path.exists():
            raise ResumeStudy.MasterFileNotFound(
                f"master file: {master_file_path} not found")

        options[master_keyword] = options[last_master_keyword]
        with open(options_file_path, "w") as options_json:
            json.dump(options, options_json)

        shutil.copy(last_master_file_path, master_file_path)

    class OptionsFileNotFound(Exception):
        pass

    class KeyWordNotFound(Exception):
        pass

    class LastMasterFileNotFound(Exception):
        pass

    class MasterFileNotFound(Exception):
        pass
