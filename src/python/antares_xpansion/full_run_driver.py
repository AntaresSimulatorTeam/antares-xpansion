
from antares_xpansion.xpansion_utils import read_and_write_mps
from antares_xpansion.benders_driver import BendersDriver
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver, ProblemGeneratorData
from antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from flushed_print import flushed_print

import os
import shutil
from pathlib import Path

# class ProblemGenerationDriverFullMode(ProblemGeneratorDriver):
#     def __init__(self, problem_generator_data: ProblemGeneratorData) -> None:
#         super().__init__(problem_generator_data)

#     def _lp_step(self):
#         # return super()._lp_step()
#         pass


# class BendersDriverFullMode(BendersDriver):
#     def __init__(self, benders_mpi, benders_sequential, merge_mps, options_file) -> None:
#         super().__init__(benders_mpi, benders_sequential, merge_mps, options_file)

#     def launch(self, simulation_output_path, method, keep_mps=False, n_mpi=1, oversubscribe=False, allow_run_as_root=False):
#         # return super().launch(simulation_output_path, method, keep_mps, n_mpi, oversubscribe, allow_run_as_root)
#         pass


class FullRunDriver:
    def __init__(self, problem_generation_driver: ProblemGeneratorDriver, benders_driver: BendersDriver) -> None:
        self.benders_driver = benders_driver
        self.problem_generation_driver = problem_generation_driver

    def launch(self, problem_generation_output_path: Path, problem_generation_is_relaxed: bool, benders_simulation_output_path, benders_method, benders_keep_mps=False, benders_n_mpi=1, benders_oversubscribe=False, benders_allow_run_as_root=False):
        """
            problem generation step : getnames + lp_namer
        """
        # Pb Gen pre-step
        self.problem_generation_driver.clear_old_log()
        self.problem_generation_driver.output_path = problem_generation_output_path

        self.problem_generation_driver.get_names()

        self.problem_generation_driver.is_relaxed = problem_generation_is_relaxed

        self.problem_generation_driver.create_lp_dir()
        self.problem_generation_driver.set_weights()

        # Benders pre-step

        self.benders_driver.method = benders_method
        self.benders_driver.n_mpi = benders_n_mpi
        self.benders_driver.oversubscribe = benders_oversubscribe
        self.benders_driver.allow_run_as_root = benders_allow_run_as_root
        self.benders_driver.simulation_output_path = benders_simulation_output_path
        old_cwd = os.getcwd()
        lp_path = self.benders_driver.get_lp_path()

        os.chdir(lp_path)
        flushed_print("Current directory is now: ", os.getcwd())
