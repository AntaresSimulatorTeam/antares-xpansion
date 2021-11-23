"""
    Class to control the Problem Generation
"""

import shutil
import os
import subprocess
import sys
from datetime import datetime

from pathlib import Path

from antares_xpansion.xpansion_utils import read_and_write_mps
from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.config_loader import ConfigLoader

import functools

flushed_print = functools.partial(print, flush=True)


class ProblemGeneratorDriver:
    def __init__(self, config_loader : ConfigLoader ) -> None:
        self.config_loader = config_loader
        self.config = self.config_loader.config
        self.options = self.config_loader.options

    def clear_old_log(self):
        if (self.config.step in ["full", "problem_generation"]) \
                and (os.path.isfile(self.config_loader.exe_path(self.config.LP_NAMER) + '.log')):
            os.remove(self.config_loader.exe_path(self.config.LP_NAMER) + '.log')

    def launch(self):
        """
            problem generation step : getnames + lp_namer
        """
        flushed_print("-- Problem Generation")
        self.get_names()
        self.lp_step()

    def get_names(self):
        """
            produces a .txt file describing the weekly problems:
            each line of the file contains :
             - mps file name
             - variables file name
             - constraints file name

            produces a file named with xpansionConfig.MPS_TXT
        """

        output_path = self.config_loader.simulation_output_path()
        mps_txt = read_and_write_mps(output_path)
        with open(os.path.normpath(os.path.join(output_path, self.config.MPS_TXT)), 'w') as file_l:
            for line in mps_txt.items():
                file_l.write(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + '\n')

        glob_path = Path(output_path)
        area_files = [str(pp) for pp in glob_path.glob("area*.txt")]
        interco_files = [str(pp) for pp in glob_path.glob("interco*.txt")]
        assert len(area_files) == 1
        assert len(interco_files) == 1
        shutil.copy(area_files[0], os.path.normpath(os.path.join(output_path, 'area.txt')))
        shutil.copy(interco_files[0], os.path.normpath(os.path.join(output_path, 'interco.txt')))


    def lp_step(self):
        """
            copies area and interco files and launches the lp_namer

            produces a file named with xpansionConfig.MPS_TXT
        """

        output_path = self.config_loader.simulation_output_path()

        lp_path = self.config_loader.simulation_lp_path()
        if os.path.isdir(lp_path):
            shutil.rmtree(lp_path)
        os.makedirs(lp_path)

        weight_file_name = self.config_loader.weight_file_name()
        if weight_file_name:
            weight_list = XpansionStudyReader.get_years_weight_from_file(self.config_loader.weights_file_path())
            YearlyWeightWriter(Path(output_path)).create_weight_file(weight_list, weight_file_name)

        with open(self.get_lp_namer_log_filename(lp_path), 'w') as output_file:

            start_time = datetime.now()
            returned_l = subprocess.run(self.get_lp_namer_command(output_path), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)

            end_time = datetime.now()
            flushed_print('Post antares step duration: {}'.format(end_time - start_time))

            if returned_l.returncode != 0:
                flushed_print("ERROR: exited lpnamer with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.config.keep_mps:
                StudyOutputCleaner.clean_lpnamer_step(Path(output_path))
        self.config_loader.set_options_for_benders_solver()


    def get_lp_namer_log_filename(self, lp_path):
        return os.path.join(lp_path, self.config.LP_NAMER + '.log')

    def get_lp_namer_command(self, output_path):
        is_relaxed = 'relaxed' if self.is_relaxed() else 'integer'
        return [self.config_loader.exe_path(self.config.LP_NAMER), "-o", str(output_path), "-f", is_relaxed, "-e",
                self.config_loader.additional_constraints()]

    def is_relaxed(self):
        """
            indicates if method to use is relaxed by reading the relaxation_type
            from the settings file
        """
        relaxation_type = self.options.get('master',
                                           self.config.settings_default["master"])
        assert relaxation_type in ['integer', 'relaxed', 'full_integer']
        return relaxation_type == 'relaxed'

