"""
    Class to control the sensitivity analysis
"""

import glob
import os
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

from antares_xpansion.logger import step_logger


class SensitivityDriver:
    def __init__(self, sensitivity_exe):
        self.sensitivity_exe = sensitivity_exe
        self.logger = step_logger(__name__, __class__.__name__)

    def launch(
        self,
        simulation_output_path,
        json_sensitivity_in_path,
        json_benders_output_path,
        last_master_path,
        last_master_basis,
        structure_path,
        json_sensitivity_out_path,
        sensitivity_log_path,
            xpansion_simulation_output
    ):
        """
        launch sensitivity analysis
        """
        self.simulation_output_path = self._get_simulation_output_path(
            simulation_output_path
        )
        # if (os.path.splitext(simulation_output_path)[1] == ".zip"):
        #     with zipfile.ZipFile(self.simulation_output_path, 'r') as output_zip:
        #         output_zip.extractall(xpansion_simulation_output)
        self.json_sensitivity_in_path = self._get_file_path(
            json_sensitivity_in_path)
        self.json_benders_output_path = self._get_file_path(
            json_benders_output_path)
        self.last_master_path = self._get_file_path(last_master_path)
        self.last_master_basis = last_master_basis
        self.structure_path = self._get_file_path(structure_path)

        self.json_sensitivity_out_path = json_sensitivity_out_path
        self.sensitivity_log_path = sensitivity_log_path

        self.logger.info("Sensitivity study")

        old_cwd = os.getcwd()
        os.chdir(xpansion_simulation_output)
        self.logger.info(f"Current directory is now {os.getcwd()}")

        returned_l = subprocess.run(
            self._get_sensitivity_cmd(),
            shell=False,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )
        if returned_l.returncode != 0:
            raise SensitivityDriver.SensitivityExeError(
                "ERROR: exited sensitivity with status %d" % returned_l.returncode
            )

        os.chdir(old_cwd)

    @staticmethod
    def _get_file_path(filepath):
        if Path(filepath).is_file():
            return filepath
        else:
            raise SensitivityDriver.SensitivityFilePathError(
                f"Sensitivity Error: {filepath} not found"
            )

    @staticmethod
    def _get_simulation_output_path(simulation_output_path):
        if not os.path.exists(simulation_output_path):
            raise SensitivityDriver.SensitivityOutputPathError(
                f"Sensitivity Error: {simulation_output_path} not found "
            )

    def _get_sensitivity_cmd(self):
        return [
            self.sensitivity_exe,
            "-i",
            self.json_sensitivity_in_path,
            "-b",
            self.json_benders_output_path,
            "-m",
            self.last_master_path,
            "-s",
            self.structure_path,
            "-o",
            self.json_sensitivity_out_path,
            "-l",
            self.sensitivity_log_path,
            "--basis",
            self.last_master_basis,
        ]

    class SensitivityOutputPathError(Exception):
        pass

    class SensitivityFilePathError(Exception):
        pass

    class SensitivityExeError(Exception):
        pass
