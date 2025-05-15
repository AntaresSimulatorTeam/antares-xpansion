"""
Class to control the Problem Generation
"""

import os
import subprocess
import sys

from dataclasses import dataclass
from pathlib import Path
from typing import List

from antares_xpansion.logger import step_logger
from antares_xpansion.study_output_cleaner import StudyOutputCleaner


@dataclass
class PresolveData:
    exe_path: Path
    keep_full_mps: bool
    # memory: bool # TODO pass in memory one day?


class PresolveDriver:
    class PresolveInputPathError(Exception):
        pass

    class PresolveExecutionError(Exception):
        pass

    class PresolveOptionsFileError(Exception):
        pass

    def __init__(self, data: PresolveData, options_file: Path) -> None:
        self.exec_path: Path = data.exe_path
        self.keep_full_mps: bool = data.keep_full_mps
        self.logger = step_logger(__name__, __class__.__name__)

        if options_file == Path():
            raise PresolveDriver.PresolveOptionsFileError("Invalid options file!")
        self.options_file = options_file

    def launch(self, simulation_output_path: Path):
        """
        presolve step
        """
        self.logger.info("Presolve")

        self.lp_path: Path = simulation_output_path / "lp"
        self.check_input_dir()

        self.logger.info(f"Switching to directory {self.lp_path}")
        old_cwd = os.getcwd()
        os.chdir(self.lp_path)
        self.clear_old_log()

        self.logger.info(f"Running {self.get_presolve_cmd()}")
        ret = subprocess.run(
            self.get_presolve_cmd(),
            shell=False,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )

        if ret.returncode != 0:
            raise PresolveDriver.PresolveExecutionError(
                f"ERROR: exited presolve with status {ret.returncode}"
            )

        elif not self.keep_full_mps:
            StudyOutputCleaner.clean_benders_step(self.lp_path / "full")

        os.chdir(old_cwd)

    def clear_old_log(self) -> None:
        if os.path.isfile(self.exec_path.name + ".log"):
            os.remove(self.exec_path.name + ".log")

    def check_input_dir(self) -> None:
        if not self.lp_path.is_dir():
            raise PresolveDriver.PresolveInputPathError(
                f"Error in lp path: {self.lp_path} not found"
            )

    def get_presolve_cmd(self) -> List[Path]:
        return [self.exec_path, self.options_file]
