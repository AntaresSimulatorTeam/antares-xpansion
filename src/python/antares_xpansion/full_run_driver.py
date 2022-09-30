
from typing import List
import subprocess
import sys
from antares_xpansion.benders_driver import BendersDriver
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver, ProblemGeneratorData
from antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.flushed_print import flushed_print

import os
import shutil
from pathlib import Path


class FullRunDriver:
    def __init__(self, full_exe, problem_generation_driver: ProblemGeneratorDriver, benders_driver: BendersDriver) -> None:
        self.full_exe = full_exe
        self.benders_driver = benders_driver
        self.problem_generation_driver = problem_generation_driver
        self.json_file_path = ""

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
        self.problem_generation_driver.output_path = output_path

        self.problem_generation_driver.get_names()

        self.problem_generation_driver.is_relaxed = problem_generation_is_relaxed

        # self.problem_generation_driver.create_lp_dir()
        self.problem_generation_driver.set_weights()

        # Benders pre-step

        self.benders_driver.method = benders_method
        self.benders_driver.n_mpi = benders_n_mpi
        self.benders_driver.oversubscribe = benders_oversubscribe
        self.benders_driver.allow_run_as_root = benders_allow_run_as_root
        self.benders_driver.simulation_output_path = output_path
        old_cwd = os.getcwd()
        lp_path = self.benders_driver.get_lp_path()

        os.chdir(lp_path)
        flushed_print("Current directory is now: ", os.getcwd())
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
        ret = subprocess.run(
            self.full_command(), shell=False, stdout=sys.stdout, stderr=sys.stderr,
            encoding='utf-8')

        if ret.returncode != 0:
            raise FullRunDriver.FullRunExecutionError(
                f"ERROR: exited solver with status {ret.returncode}"
            )

    def full_command(self) -> List:
        bare_solver_command = [
            self.full_exe, "--benders_options", self.benders_driver.options_file, "--method", self.benders_driver.method, "-s",
            str(self.json_file_path)]
        bare_solver_command.extend(
            self.problem_generation_driver.lp_namer_options())

        if self.benders_driver.solver == self.benders_driver.benders_mpi:
            mpi_command = self.benders_driver.get_mpi_run_command_root()
            mpi_command.extend(bare_solver_command)
            return mpi_command
        else:
            return bare_solver_command

    class FullRunExecutionError(Exception):
        pass
