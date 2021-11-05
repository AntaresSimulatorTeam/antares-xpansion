"""
    Class to control the study update
"""

import os
import subprocess
import sys
from pathlib import Path
from dataclasses import dataclass

from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.flushed_print import flushed_print


class StudyUpdaterDriverException :
    class BaseException(Exception):
        pass
    class StudyUpdaterOutputPathError(BaseException):
        pass

@dataclass
class StudyUpdaterData:
    study_updater_exe : str
    simulation_output_path : Path
    JSON_NAME : str
    keep_mps : bool

class StudyUpdaterDriverException :
    class BaseException(Exception):
        pass
    class StudyUpdaterOutputPathError(BaseException):
        pass

@dataclass
class StudyUpdaterData:
    study_updater_exe : str
    simulation_output_path : Path
    JSON_NAME : str
    keep_mps : bool

class StudyUpdaterDriver:
    def __init__(self, study_updater_data: StudyUpdaterData) -> None:
        
        self.simulation_output_path =  study_updater_data.simulation_output_path
        self.study_updater_exe =  study_updater_data.study_updater_exe
        self.JSON_NAME         =  study_updater_data.JSON_NAME
        self.keep_mps          =  study_updater_data.keep_mps


    def set_simulation_output_path(self, simulation_output_path : Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else : 
            raise StudyUpdaterDriverException.StudyUpdaterOutputPathError(f"Study Updater Error: {simulation_output_path} not found ")

    def get_simulation_output_path(self):
        return self._simulation_output_path

    def get_study_updater_exe(self):
        return self._study_updater_exe

    def set_study_updater_exe(self, study_updater_exe : str):
        if Path(study_updater_exe).is_file():
            self._study_updater_exe = study_updater_exe
        else : 
            raise StudyUpdaterDriverException.StudyUpdaterOutputPathError(f"Study Updater Error: {study_updater_exe} not found ")

    def _clear_old_log(self):
        if os.path.isfile(self.study_updater_exe + '.log'):
            os.remove(self.study_updater_exe + '.log')


    def launch(self):
        """
            updates the antares study using the candidates file and the json solution output

        """
        print ("-- Study Update")
        self._clear_old_log()

        with open(self.get_study_updater_log_filename(), 'w') as output_file:
            returned_l = subprocess.run(self.get_study_updater_command(), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)
            if returned_l.returncode != 0:
                print("ERROR: exited study-updater with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.keep_mps:
                StudyOutputCleaner.clean_study_update_step(self.simulation_output_path)

    def get_study_updater_log_filename(self):
        return os.path.join(self.simulation_output_path, self.study_updater_exe + '.log')

    def get_study_updater_command(self):
        return [self.study_updater_exe, "-o", str(self.simulation_output_path), "-s",
                self.JSON_NAME + ".json"]
                
    
    simulation_output_path = property(get_simulation_output_path, set_simulation_output_path)            
    study_updater_exe      = property(get_study_updater_exe, set_study_updater_exe)            
