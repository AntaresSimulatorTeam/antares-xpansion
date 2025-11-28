"""
Class to control the execution of the trajectory investment
"""

import os
from pathlib import Path

from antares_xpansion.logger import step_logger
from antares_xpansion.trajectory.driver_input_translation import InputTranslationDriver
from antares_xpansion.trajectory.driver_merge_master import (
    MergeMasterData,
    MergeMasterDriver,
)
from antares_xpansion.trajectory.driver_merge_weights import (
    MergeWeightsData,
    MergeWeightsDriver,
)
from antares_xpansion.trajectory.driver_multiple_problem_gen import (
    MultipleProblemGenerationData,
    MultipleProblemGenerationDriver,
)
from antares_xpansion.trajectory.driver_resolution import (
    TrajectoryResolutionData,
    TrajectoryResolutionDriver,
)
from antares_xpansion.trajectory.trajectory_config import TrajectoryConfig


class TrajectoryInvestmentDriver:
    """
    Class to control and launch investment studies on a trajectory
    """

    def __init__(self, config: TrajectoryConfig):
        self.config = config
        self.logger = step_logger(__name__, __class__.__name__)

        # Change the working directory to the input root.
        previous_dir = os.getcwd()
        os.chdir(self.config.input_root)

        # Create the intermediary folder
        self.intermediary_folder_path = self.prepare_folder(
            self.config.INTERMEDIARY_FOLDER
        )
        # Prepare intermediary file names
        self.master_merger_info_file = (
                self.intermediary_folder_path / self.config.MASTER_MERGER_INFO_FILE
        )
        self.nodal_lp_info_file = (
                self.intermediary_folder_path / self.config.NODAL_LP_INFO_FILE
        )
        self.merged_weights_file = self.config.input_root / self.config.MERGED_WEIGHTS

        # We leave the default values for where to write intermediary files
        mpg_data = MultipleProblemGenerationData(
            self.config.get_executable_path(self.config.MULTIPLE_PROBLEM_GEN),
            self.config.input_root,
            self.config.input_file,
            self.config.memory,
            self.intermediary_folder_path / self.config.MPG_INPUT_FILE,
            self.intermediary_folder_path / self.config.MPG_WEIGHTS_FILE,
            self.intermediary_folder_path / self.config.MPG_CONSTRAINTS_FILE,
            self.nodal_lp_info_file,
        )
        self.mpg_driver = MultipleProblemGenerationDriver(mpg_data)

        # Input translation driver
        self.input_translation_driver = InputTranslationDriver(
            self.config.input_root,
            self.config.input_file,
            self.master_merger_info_file,
        )

        # We leave the default values for where to write intermediary files
        self.output_folder = self.prepare_folder(self.config.OUTPUT_FOLDER)
        # The problems format and solver are expected as full upper case by the C++ executables
        # ONE
        solver = self.config.solver
        problems_format = self.config.problems_format
        # TODO : hardcoded solver for now
        # TWO
        solver = "Xpress"
        problems_format = "OPTIMIZED"
        if solver != "Xpress":
            problems_format = "MPS"

        mm_data = MergeMasterData(
            self.config.get_executable_path(self.config.MERGE_MASTER_MPS),
            self.master_merger_info_file,
            self.nodal_lp_info_file,
            self.intermediary_folder_path / self.config.MERGE_MASTER_OPTIONS_FILE,
            self.config.input_root,
            self.output_folder,
            solver,
            problems_format,
            self.config.MERGED_MASTER,
            self.config.MERGED_STRUCTURE,
        )
        self.merge_master_driver = MergeMasterDriver(mm_data)

        # We leave the default values for where to write intermediary files
        mw_data = MergeWeightsData(
            self.config.get_executable_path(self.config.MERGE_WEIGHTS),
            self.master_merger_info_file,
            self.config.input_root,
            self.nodal_lp_info_file,
            self.merged_weights_file,
        )
        self.merge_weights_driver = MergeWeightsDriver(mw_data)

        # Prepare a ConfigLoader object. Used for the resolution, when using the benders driver
        benders_options_file = self.intermediary_folder_path / "options_benders.json"
        benders_json_output = self.output_folder / "out_benders.json"
        root_study = self.input_translation_driver.get_root_study(
            self.config.input_root
        )

        res_data = TrajectoryResolutionData(
            benders_exe=self.config.get_executable_path(self.config.BENDERS),
            frontal_exe=self.config.get_executable_path(self.config.MERGE_MPS),
            outer_loop_exe=self.config.get_executable_path(self.config.OUTER_LOOP),
            mpi_exe=self.config.get_executable_path(Path(self.config.MPIEXEC).name),
            input_root=self.config.input_root,
            root_study=root_study,
            json_output_file=benders_json_output,
            benders_options_file=benders_options_file,
            merged_weights_file=self.merged_weights_file,
            output_folder=self.output_folder,
            master_name=self.config.MERGED_MASTER,
            structure_file=self.config.MERGED_STRUCTURE,
            solver=solver,
            problems_format=problems_format,
            cache_problems=self.config.cache_problems,
            method=self.config.method,
            n_mpi=self.config.n_mpi,
            oversubscribe=self.config.oversubsribe,
            allow_run_as_root=self.config.allow_run_as_root,
            master_formulation=self.input_translation_driver.get_master_formulation(),
        )

        self.resolution_driver = TrajectoryResolutionDriver(res_data)

        # Change back the directory
        os.chdir(previous_dir)

    def prepare_folder(self, name: str):
        """
        Creates the folder at './<name>' and returns the full path
        This assumes we are working at the input root.
        """
        assert Path(os.getcwd()).resolve() == self.config.input_root.resolve()
        folder = Path(f"./{name}")
        if not folder.is_dir():
            os.makedirs(folder)

        return folder

    def launch(self):
        if self.config.step == "full":
            self.logger.info("Launching full procedure.")
            self.input_translation_driver.launch()
            self.mpg_driver.launch()
            self.merge_master_driver.launch()
            self.merge_weights_driver.launch()
            self.resolution_driver.launch()

        elif self.config.step == "input_translation":
            self.logger.info("Verifying and translating user input.")
            self.input_translation_driver.launch()

        elif self.config.step == "problem_generation":
            self.logger.info("Running problem generation on the studies in the tree.")
            self.mpg_driver.launch()

        elif self.config.step == "merge_master":
            self.logger.info("Merging the nodal master problems.")
            self.merge_master_driver.launch()

        elif self.config.step == "merge_weights":
            self.logger.info("Generating a merged weights file.")
            self.merge_weights_driver.launch()

        elif self.config.step == "resolution":
            self.logger.info("Launching the resolution")
            self.resolution_driver.launch()
