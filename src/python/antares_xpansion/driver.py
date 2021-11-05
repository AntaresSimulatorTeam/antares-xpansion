"""
    Class to control the execution of the optimization session
"""
import sys

from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver
from antares_xpansion.benders_driver import BendersDriver
from antares_xpansion.study_updater_driver import StudyUpdaterDriver, StudyUpdaterData

import functools

print = functools.partial(print, flush=True)


class XpansionDriver:
    """
        Class to control the execution of the optimization session
    """

    def __init__(self, config):
        """
            Initialise driver with a given antaresXpansion configuration,
            the system platform and parses the arguments
            :param config: configuration to use for the optimization
            :type config: XpansionConfig object
        """
        self.platform = sys.platform
        self.config = config

        self.config_loader = ConfigLoader(self.config)
        self.antares_driver = AntaresDriver(self.config_loader)
        self.problem_generator_driver = ProblemGeneratorDriver(self.config_loader)
        self.benders_driver = BendersDriver(self.config_loader)
        self.study_update_driver = StudyUpdaterDriver(StudyUpdaterData(study_updater_exe = self.config_loader.exe_path(self.config.STUDY_UPDATER),
                                                                       simulation_output_path = self.config_loader.simulation_output_path(),
                                                                       JSON_NAME = self.config.options_default["JSON_NAME"],
                                                                       keep_mps = self.config.keep_mps))

    def launch(self):
        """
            launch antares xpansion steps
        """
        self._clear_old_log()

        if self.config.step == "full":
            self.antares_driver.launch()
            print("-- post antares")
            self.problem_generator_driver.launch()
            self.benders_driver.launch()
            self.study_update_driver.launch()
        elif self.config.step == "antares":
            self.antares_driver.launch()
        elif self.config.step == "problem_generation":
            if self.config.simulation_name:
                self.problem_generator_driver.launch()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "study_update":
            if self.config.simulation_name:
                self.study_update_driver.launch()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "benders":
            if self.config.simulation_name:
                self.benders_driver.launch()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        else:
            print("Launching failed")
            sys.exit(1)

    def _clear_old_log(self):
        self.antares_driver.clear_old_log()
        self.problem_generator_driver.clear_old_log()
