"""
    Class to control the Problem Generation
"""

import os
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List

from antares_xpansion.logger import step_logger


@dataclass
class ProblemGeneratorData:
    keep_mps: bool
    additional_constraints: str
    user_weights_file_path: Path
    weight_file_name_for_lp: str
    lp_namer_exe_path: Path
    active_years: List
    memory: bool


class ProblemGeneratorDriver:
    class BasicException(Exception):
        pass

    class MpsZipFileException(BasicException):
        pass

    class AreaFileException(BasicException):
        pass

    class IntercoFilesException(BasicException):
        pass

    class OutputPathError(BasicException):
        pass

    class LPNamerExeError(BasicException):
        pass

    class LPNamerPathError(BasicException):
        pass

    class LPNamerExecutionError(BasicException):
        pass

    def __init__(self, problem_generator_data: ProblemGeneratorData) -> None:

        self.lp_namer_exe_path = Path(problem_generator_data.lp_namer_exe_path)
        self.LP_NAMER = self.lp_namer_exe_path.name
        self.keep_mps = problem_generator_data.keep_mps
        self.additional_constraints = problem_generator_data.additional_constraints
        self.user_weights_file_path = problem_generator_data.user_weights_file_path
        self.weight_file_name_for_lp = problem_generator_data.weight_file_name_for_lp
        self.active_years = problem_generator_data.active_years
        self.MPS_TXT = "mps.txt"
        self.is_relaxed = False
        self._lp_path = None
        self.logger = step_logger(__name__, __class__.__name__)
        self.memory = problem_generator_data.memory

    def launch(self, output_path: Path, is_relaxed: bool):
        """
            problem generation step : getnames + lp_namer
        """
        self.clear_old_log()
        self.logger.info("Problem Generation")
        self.output_path = output_path

        self.create_lp_dir()

        self.is_relaxed = is_relaxed
        self._lp_step()

    def launch_memory(self, study_path: Path, is_relaxed: bool):
        """
            problem generation step : getnames + lp_namer
        """
        self.logger.info("Problem Generation")
        self.study_path = study_path
        self.is_relaxed = is_relaxed
        self._lp_step()

    def set_output_path(self, output_path):

        if output_path.exists():
            self._output_path = output_path
            self.xpansion_output_dir = output_path.parent / \
                                       (output_path.stem + "-Xpansion")
            if self.xpansion_output_dir.exists():
                shutil.rmtree(self.xpansion_output_dir)
            os.makedirs(self.xpansion_output_dir)
            self._lp_path = os.path.normpath(
                os.path.join(self.xpansion_output_dir, 'lp'))
        else:
            raise ProblemGeneratorDriver.OutputPathError(
                f"{output_path} not found")

    def get_output_path(self):
        return self._output_path

    def clear_old_log(self):
        if (os.path.isfile(str(self.lp_namer_exe_path) + '.log')):
            os.remove(str(self.lp_namer_exe_path) + '.log')

    def _lp_step(self):
        """
            copies area and interco files and launches the lp_namer

            produces a file named with xpansionConfig.MPS_TXT
        """

        returned_l = subprocess.run(self._get_lp_namer_command(), shell=False,
                                    stdout=sys.stdout, stderr=sys.stderr)

        if returned_l.returncode != 0:
            raise ProblemGeneratorDriver.LPNamerExecutionError(
                "ERROR: exited lpnamer with status %d" % returned_l.returncode)

    def create_lp_dir(self):
        if os.path.isdir(self._lp_path):
            shutil.rmtree(self._lp_path)
        os.makedirs(self._lp_path)

    def lp_namer_options(self):
        is_relaxed = 'relaxed' if self.is_relaxed else 'integer'
        if self.memory:
            ret = ["--study", str(os.path.normpath(self.study_path)), "-f", is_relaxed]  # study/output/xpansion_output_dir
        else:
            ret = ["-a", str(self.output_path), "-f", is_relaxed]
        if self.weight_file_name_for_lp:
            ret.extend(["-w", str(self.user_weights_file_path)])

        if self.additional_constraints != "":
            ret.extend(["-e",
                        self.additional_constraints])

        return ret

    def _get_lp_namer_command(self):

        if not self.lp_namer_exe_path.is_file():
            raise ProblemGeneratorDriver.LPNamerExeError(
                f"LP namer exe: {self.lp_namer_exe_path} not found")
        command = [self.lp_namer_exe_path]
        command.extend(self.lp_namer_options())
        return command

    output_path = property(get_output_path, set_output_path)
