"""
    Class to control the execution of the antares step
"""

import os
import subprocess
from datetime import datetime

from pathlib import Path


from antares_xpansion.general_data_reader import IniReader
from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.config_loader import ConfigLoader

import functools

print = functools.partial(print, flush=True)


class AntaresDriver:
    def __init__(self, config_loader : ConfigLoader) -> None:
        self.config_loader = config_loader
        self.config = self.config_loader.config
        self.options = self.config_loader.options

    def clear_old_log(self):
        if (self.config.step in ["full", "antares"]) and (os.path.isfile(self.antares() + '.log')):
            os.remove(self.antares() + '.log')

    def antares(self):
        """
            returns antares binaries location
        """
        return self.config_loader.exe_path(self.config.ANTARES)

    def _antares_step(self):
        self._change_general_data_file_to_configure_antares_execution()
        self.launch_antares()

    def launch(self):
        self._antares_step()

    def _change_general_data_file_to_configure_antares_execution(self):
        print("-- pre antares")
        with open(self.config_loader.general_data(), 'r') as reader:
            lines = reader.readlines()

        with open(self.config_loader.general_data(), 'w') as writer:
            current_section = ""
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    line = self._get_new_line(line, current_section, key)
                else:
                    current_section = line.strip()

                if line:
                    writer.write(line)

    def _get_new_line(self, line, section, key):
        changed_val = self._get_values_to_change_general_data_file()
        if (section, key) in changed_val:
            new_val = changed_val[(section, key)]
            if new_val:
                line = key + ' = ' + new_val + '\n'
            else:
                line = None
        return line


    def _get_values_to_change_general_data_file(self):
        return {('[' + self.config.OPTIMIZATION + ']', self.config.EXPORT_MPS): 'true',
                ('[' + self.config.OPTIMIZATION + ']', self.config.EXPORT_STRUCTURE): 'true',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-tc-minstablepower'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-tc-min-ud-time'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-dayahead'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']', self.config.USE_XPRS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.INBASIS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.OUTBASIS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.TRACE): None,
                ('[general]', 'mode'): 'expansion' if self.is_accurate() else 'Economy',
                (
                    '[other preferences]',
                    'unit-commitment-mode'): 'accurate' if self.is_accurate() else 'fast'
                }
    def is_accurate(self):
        """
            indicates if method to use is accurate by reading the uc_type in the settings file
        """
        uc_type = self.options.get(self.config.UC_TYPE,
                                   self.config.settings_default[self.config.UC_TYPE])
        assert uc_type in [self.config.EXPANSION_ACCURATE, self.config.EXPANSION_FAST]
        return uc_type == self.config.EXPANSION_ACCURATE


    def launch_antares(self):
        print("-- launching antares")
        simulation_name = ""

        if not os.path.isdir(self.config_loader.antares_output()):
            os.mkdir(self.config_loader.antares_output())
        old_output = os.listdir(self.config_loader.antares_output())

        start_time = datetime.now()

        returned_l = subprocess.run(self.get_antares_cmd(), shell=False,
                                    stdout=subprocess.DEVNULL,
                                    stderr=subprocess.DEVNULL)

        end_time = datetime.now()
        print('Antares simulation duration : {}'.format(end_time - start_time))

        if returned_l.returncode != 0:
            print("WARNING: exited antares with status %d" % returned_l.returncode)
        else:
            new_output = os.listdir(self.config_loader.antares_output())
            assert len(old_output) + 1 == len(new_output)
            diff = list(set(new_output) - set(old_output))
            simulation_name = str(diff[0])
            StudyOutputCleaner.clean_antares_step((Path(self.config_loader.antares_output()) / simulation_name))

        self.simulation_name = simulation_name


    def get_antares_cmd(self):
        return [self.antares(), self.config_loader.data_dir()]

