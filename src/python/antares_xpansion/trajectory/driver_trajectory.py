"""
Class to control the execution of the trajectory investment
"""

from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters
from antares_xpansion.logger import step_logger
from antares_xpansion.trajectory.driver_multiple_problem_gen import (
    MultipleProblemGenerationData,
    MultipleProblemGenerationDriver,
)
from antares_xpansion.trajectory.driver_merge_master import (
    MergeMasterData,
    MergeMasterDriver,
)
from antares_xpansion.trajectory.driver_input_translation import InputTranslationDriver
from antares_xpansion.trajectory.trajectory_config import TrajectoryConfig

from antares_xpansion.xpansionConfig import ConfigParameters

import os
from pathlib import Path


class TrajectoryInvestmentDriver:
    """
    Class to control and launch investment studies on a trajectory
    """

    def __init__(self, config: TrajectoryConfig):
        self.config = config
        self.logger = step_logger(__name__, __class__.__name__)

        # Create the intermediary folder
        self.intermediary_folder_path = (
            self.config.input_root / self.config.INTERMEDIARY_FOLDER
        )
        if not self.intermediary_folder_path.is_dir():
            os.makedirs(self.intermediary_folder_path)

        # We leave the default values for where to write intermediary files
        mpg_data = MultipleProblemGenerationData(
            Path(config.default_install_dir) / self.config.MULTIPLE_PROBLEM_GEN,
            self.config.input_root,
            self.config.input_file,
            self.config.memory,
            self.intermediary_folder_path / self.config.MPG_INPUT_FILE,
            self.intermediary_folder_path / self.config.MPG_WEIGHTS_FILE,
            self.intermediary_folder_path / self.config.MPG_CONSTRAINTS_FILE,
            self.intermediary_folder_path / self.config.NODAL_LP_INFO_FILE,
        )
        self.mpg_driver = MultipleProblemGenerationDriver(mpg_data)

        # Input translation driver
        self.input_translation_driver = InputTranslationDriver(
            self.config.input_file,
            self.intermediary_folder_path / self.config.MASTER_MERGER_INFO_FILE,
        )

        # We leave the default values for where to write intermediary files
        output_folder = self.config.input_root / "output"
        if not output_folder.is_dir():
            os.makedirs(output_folder)
        # TODO : hardcoded solver for now
        solver = "XPRESS"
        problems_format = "SAVED"
        if solver != "XPRESS":
            problems_format = "MPS"

        mm_data = MergeMasterData(
            Path(self.config.default_install_dir) / self.config.MERGE_MASTER_MPS,
            self.intermediary_folder_path / self.config.MASTER_MERGER_INFO_FILE,
            self.intermediary_folder_path / self.config.NODAL_LP_INFO_FILE,
            self.intermediary_folder_path / self.config.MERGE_MASTER_OPTIONS_FILE,
            self.config.input_root,
            output_folder,
            solver,
            problems_format,
            self.config.MERGED_MASTER,
            self.config.MERGED_STRUCTURE,
        )
        self.merge_master_driver = MergeMasterDriver(mm_data)

    def launch(self):
        if self.config.step == "full":
            self.logger.info("Launching full procedure.")

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

        elif self.config.step == "resolution":
            self.logger.info("Launching the resolution")
