"""
    Class to control the study update
"""

import os
import subprocess
import sys

from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.config_loader import ConfigLoader


class StudyUpdaterDriver:
    def __init__(self, config_loader : ConfigLoader) -> None:
        self.config_loader = config_loader
        self.config = self.config_loader.config
        self.options = self.config_loader.options

    def clear_old_log(self):
        if (self.config.step in ["full", "study_update"]) \
                and (os.path.isfile(self.config_loader.exe_path(self.config.STUDY_UPDATER) + '.log')):
            os.remove(self.config_loader.exe_path(self.config.STUDY_UPDATER) + '.log')

    def launch(self):
        self.update_step()

    def update_step(self):
        """
            updates the antares study using the candidates file and the json solution output

        """
        print ("-- Study Update")
        output_path = self.config_loader.simulation_output_path()

        with open(self.get_study_updater_log_filename(), 'w') as output_file:
            returned_l = subprocess.run(self.get_study_updater_command(output_path), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)
            if returned_l.returncode != 0:
                print("ERROR: exited study-updater with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.config.keep_mps:
                StudyOutputCleaner.clean_study_update_step(output_path)

    def get_study_updater_log_filename(self):
        return os.path.join(self.config_loader.simulation_output_path(), self.config.STUDY_UPDATER + '.log')

    def get_study_updater_command(self, output_path):
        return [self.config_loader.exe_path(self.config.STUDY_UPDATER), "-o", output_path, "-s",
                self.config.options_default["JSON_NAME"] + ".json"]
                