"""
    Class to control the study update
"""

import os
import subprocess
import sys
from pathlib import Path
from dataclasses import dataclass

from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.flushed_print import flushed_print


class StudyUpdaterDriver:
    def __init__(self, study_updater_exe, json_name) -> None:

        self._set_study_updater_exe(study_updater_exe)
        self.JSON_NAME = json_name

    def _set_study_updater_exe(self, study_updater_exe: str):
        if Path(study_updater_exe).is_file():
            self.study_updater_exe = study_updater_exe
        else:
            raise StudyUpdaterDriver.StudyUpdaterOutputPathError(
                f"Study Updater Error: {study_updater_exe} not found ")

    def launch(self, simulation_output_path, keep_mps=False):
        """
            updates the antares study using the candidates file and the json solution output

        """
        self._set_simulation_output_path(simulation_output_path)
        flushed_print("-- Study Update")
        self._clear_old_log()

        with open(self.get_study_updater_log_filename(), 'w') as output_file:
            returned_l = subprocess.run(self.get_study_updater_command(), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)
            if returned_l.returncode != 0:
                raise StudyUpdaterDriver.UpdaterExecutionError(
                    f"ERROR: exited study-updater with status {returned_l.returncode}")

            elif not keep_mps:
                StudyOutputCleaner.clean_study_update_step(
                    self.simulation_output_path)

    def _set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self.simulation_output_path = simulation_output_path
        else:
            raise StudyUpdaterDriver.StudyUpdaterOutputPathError(
                f"Study Updater Error: {simulation_output_path} not found ")

    def _clear_old_log(self):
        if os.path.isfile(self.study_updater_exe + '.log'):
            os.remove(self.study_updater_exe + '.log')

    def get_study_updater_log_filename(self):
        return os.path.join(self.simulation_output_path, self.study_updater_exe + '.log')

    def get_study_updater_command(self):
        return [self.study_updater_exe, "-o", str(self.simulation_output_path), "-s",
                self.JSON_NAME + ".json"]

    class UpdaterExecutionError(Exception):
        pass

    class StudyUpdaterOutputPathError(Exception):
        pass
