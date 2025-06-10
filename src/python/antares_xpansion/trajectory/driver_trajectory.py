"""
Class to control the execution of the trajectory investment
"""

from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters
from antares_xpansion.logger import step_logger
from antares_xpansion.trajectory.driver_multiple_problem_gen import (
    MultipleProblemGenerationData,
    MultipleProblemGenerationDriver,
)
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
        data = MultipleProblemGenerationData(
            Path(config.default_install_dir) / config.MULTIPLE_PROBLEM_GEN,
            self.config.input_root,
            self.config.input_file,
            self.config.memory,
            self.intermediary_folder_path / self.config.MPG_INPUT_FILE,
            self.intermediary_folder_path / self.config.MPG_WEIGHTS_FILE,
            self.intermediary_folder_path / self.config.MPG_CONSTRAINTS_FILE,
            self.intermediary_folder_path / self.config.NODAL_LP_INFO_FILE,
        )
        self.mpg_driver = MultipleProblemGenerationDriver(data)

    def launch(self):
        if self.config.step == "full":
            self.logger.info("Launching full procedure.")

        elif self.config.step == "input_translation":
            self.logger.info("Verifying and translating user input.")

        elif self.config.step == "problem_generation":
            self.logger.info("Running problem generation on the studies in the tree.")
            self.mpg_driver.launch()

        elif self.config.step == "merge_master":
            self.logger.info("Merging the nodal master problems.")

        elif self.config.step == "merge_weights":
            self.logger.info("Generating a merged weights file.")

        elif self.config.step == "resolution":
            self.logger.info("Launching the resolution")
