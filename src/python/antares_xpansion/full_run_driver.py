import os
import subprocess
import sys
from pathlib import Path
from typing import List

from antares_xpansion.benders_driver import BendersDriver
from antares_xpansion.logger import step_logger
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver
from antares_xpansion.study_output_cleaner import StudyOutputCleaner


class FullRunDriver:
    def __init__(self, full_exe, problem_generation_driver: ProblemGeneratorDriver, benders_driver: BendersDriver) -> None:
        self.full_exe = full_exe
        self.benders_driver = benders_driver
        self.problem_generation_driver = problem_generation_driver
        self.json_file_path = ""
        self.logger = step_logger(__name__, __class__.__name__)

    def prepare_drivers(self, output_path: Path,
                        problem_generation_is_relaxed: bool,
                        benders_method,
                        json_file_path,
                        benders_keep_mps=False,
                        benders_n_mpi=1,
                        benders_oversubscribe=False,
                        benders_allow_run_as_root=False):
        """
            problem generation step : getnames + lp_namer
        """
        # Pb Gen pre-step
        self.problem_generation_driver.clear_old_log()

        self.problem_generation_driver.is_relaxed = problem_generation_is_relaxed

        self.keep_mps = benders_keep_mps
        # Benders pre-step

        self.benders_driver.method = benders_method
        self.benders_driver.n_mpi = benders_n_mpi
        self.benders_driver.oversubscribe = benders_oversubscribe
        self.benders_driver.allow_run_as_root = benders_allow_run_as_root
        self.benders_driver.simulation_output_path = self.problem_generation_driver.xpansion_output_dir
        self.benders_driver.set_solver()

        self.json_file_path = json_file_path

    def launch(self,  output_path: Path,
               problem_generation_is_relaxed: bool,
               benders_method,
               json_file_path,
               benders_keep_mps=False,
               benders_n_mpi=1,
               benders_oversubscribe=False,
               benders_allow_run_as_root=False):
        self.prepare_drivers(
            output_path, problem_generation_is_relaxed, benders_method,
            json_file_path, benders_keep_mps, benders_n_mpi, benders_oversubscribe, benders_allow_run_as_root)
        self.run()

    def run(self):
        old_cwd = os.getcwd()

        lp_path = self.benders_driver.get_lp_path()

        os.chdir(lp_path)
        self.logger.info(f"Current directory is now: {os.getcwd()}")
        self.logger.info(f"Command is {self.full_command()}")
        ret = subprocess.run(
            self.full_command(), shell=False, stdout=sys.stdout, stderr=sys.stderr,
            encoding='utf-8')

        if ret.returncode != 0:
            raise FullRunDriver.FullRunExecutionError(
                f"ERROR: exited {self.full_exe} with status {ret.returncode}"
            )
        elif not self.keep_mps:
            StudyOutputCleaner.clean_benders_step(
                self.benders_driver.simulation_output_path)
        os.chdir(old_cwd)

    def full_command(self) -> List:
        bare_solver_command = [
            self.full_exe, "--benders_options", self.benders_driver.options_file, "--method", self.benders_driver.method, "-s",
            str(self.json_file_path)]
        bare_solver_command.extend(
            self.problem_generation_driver.lp_namer_options())

        if (self.benders_driver.solver in [self.benders_driver.benders, self.benders_driver.benders_by_batch]) and self.benders_driver.n_mpi > 1:
            mpi_command = self.benders_driver.get_mpi_run_command_root()
            mpi_command.extend(bare_solver_command)
            return mpi_command
        else:
            return bare_solver_command

    class FullRunExecutionError(Exception):
        pass
