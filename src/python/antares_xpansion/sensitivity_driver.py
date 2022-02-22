"""
    Class to control the sensitivity analysis
"""

import os
from pathlib import Path
import subprocess
import sys

from antares_xpansion.flushed_print import flushed_print


class SensitivityDriver:
    def __init__(self, sensitivity_exe):
        self.sensitivity_exe = sensitivity_exe

    def launch(
        self,
        simulation_output_path,
        json_sensitivity_in_path,
        json_sensitivity_out_path,
    ):
        """
        launch sensitivity analysis
        """
        self._set_simulation_output_path(simulation_output_path)
        self._set_json_input_file_path(json_sensitivity_in_path)

        self.json_sensitivity_out_path = json_sensitivity_out_path

        flushed_print("-- Sensitivity analysis")

        old_cwd = os.getcwd()
        os.chdir(simulation_output_path)
        flushed_print(f"Current directory is now {os.getcwd()}")

        returned_l = subprocess.run(
            self._get_sensitivity_cmd(), shell=False, stdout=sys.stdout, stderr=sys.stderr
        )
        if returned_l.returncode != 0:
            raise SensitivityDriver.SensitivityExeError(
                "ERROR: exited sensitivity with status %d" % returned_l.returncode
            )

        os.chdir(old_cwd)

    def _set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self.simulation_output_path = simulation_output_path
        else:
            raise SensitivityDriver.SensitivityOutputPathError(
                f"Sensitivity Error: {simulation_output_path} not found "
            )

    def _set_json_input_file_path(self, json_sensitivity_in_path):
        if Path(json_sensitivity_in_path).is_file():
            self.json_sensitivity_in_path = json_sensitivity_in_path
        else:
            raise SensitivityDriver.SensitivityJsonFilePathError(
                f"Sensitivity Error: {json_sensitivity_in_path} not found "
            )

    def _get_sensitivity_cmd(self):
        return [
            self.sensitivity_exe,
            "-j",
            self.json_sensitivity_in_path,
            "-o",
            self.json_sensitivity_out_path,
        ]

    class SensitivityOutputPathError(Exception):
        pass

    class SensitivityJsonFilePathError(Exception):
        pass

    class SensitivityExeError(Exception):
        pass
