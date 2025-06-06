"""
Class to control the execution of the trajectory investment
"""

from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters
from antares_xpansion.logger import step_logger


class TrajectoryInvestmentDriver:
    """
    Class to control and launch investment studies on a trajectory
    """

    def __init__(self, input_parameters: TrajectoryInputParameters):
        self.input_parameters = input_parameters
        self.logger = step_logger(__name__, __class__.__name__)

    def launch(self):
        if self.input_parameters.step == "full":
            self.logger.info("Launching full procedure.")

        elif self.input_parameters.step == "input_translation":
            self.logger.info("Verifying and translating user input.")

        elif self.input_parameters.step == "problem_generation":
            self.logger.info("Running problem generation on the studies in the tree.")

        elif self.input_parameters.step == "merge_master":
            self.logger.info("Merging the nodal master problems.")

        elif self.input_parameters.step == "merge_weights":
            self.logger.info("Generating a merged weights file.")

        elif self.input_parameters.step == "resolution":
            self.logger.info("Launching the resolution")
