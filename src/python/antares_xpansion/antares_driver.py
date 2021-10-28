"""
    Class to control the execution of the antares step
"""

import os
import subprocess
from datetime import datetime
from pathlib import Path


from antares_xpansion.general_data_reader import IniReader
from antares_xpansion.study_output_cleaner import StudyOutputCleaner

import functools

print = functools.partial(print, flush=True)

    
class AntaresDriver:
    def __init__(self, antares_exe_path: Path) -> None:
        
        self.antares_exe_path = antares_exe_path
        #antares study dir given at launch time 
        self.data_dir =  "" 

        self.settings = 'settings'
        self.general_data_ini = 'generaldata.ini'
        self.output = 'output'

        self.is_accurate = False
        
    def launch_accurate_mode(self, antares_study_path):
        self.is_accurate = True
        self._launch(antares_study_path)  

    def launch_fast_mode(self, antares_study_path):
        self.is_accurate = False
        self._launch(antares_study_path)  

    def _launch(self, antares_study_path):
        self._clear_old_log()
        self.data_dir = antares_study_path
        self._change_general_data_file_to_configure_antares_execution()
        self.launch_antares()

    def _clear_old_log(self):
        if os.path.isfile(self.antares_exe_path + '.log'):
            os.remove(self.antares_exe_path + '.log')

    def _change_general_data_file_to_configure_antares_execution(self):
        print("-- pre antares")
        with open(self._general_data_ini_file_path(), 'r') as reader:
            lines = reader.readlines()

        with open(self._general_data_ini_file_path(), 'w') as writer:
            current_section = ""
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    line = self._get_new_line(line, current_section, key)
                else:
                    current_section = line.strip()

                if line:
                    writer.write(line)

    def _general_data_ini_file_path(self):
        """
            returns path to general data ini file
        """
        return os.path.normpath(os.path.join(self.data_dir,
                                             self.settings, self.general_data_ini))

    def antares_output_dir(self):
        """
            returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir, self.output))                                             

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
        optimization = '[optimization]'

        return {(optimization, 'include-exportmps'): 'true',
                (optimization, 'include-exportstructure'): 'true',
                (optimization, 'include-tc-minstablepower'): 'true' if self.is_accurate else 'false',
                (optimization,'include-tc-min-ud-time'): 'true' if self.is_accurate else 'false',
                (optimization,'include-dayahead'): 'true' if self.is_accurate else 'false',
                (optimization, 'include-usexprs'): None,
                (optimization, 'include-inbasis'):  None,
                (optimization, 'include-outbasis'): None,
                (optimization, 'include-trace'):    None,
                ('[general]', 'mode'): 'expansion' if self.is_accurate else 'Economy',
                (
                    '[other preferences]',
                    'unit-commitment-mode'): 'accurate' if self.is_accurate else 'fast'
                }



    def launch_antares(self):
        print("-- launching antares")
        simulation_name = ""

        if not os.path.isdir(self.antares_output_dir()):
            os.mkdir(self.antares_output_dir())
        old_output = os.listdir(self.antares_output_dir())

        start_time = datetime.now()

        returned_l = subprocess.run(self.get_antares_cmd(), shell=False,
                                    stdout=subprocess.DEVNULL,
                                    stderr=subprocess.DEVNULL)

        end_time = datetime.now()
        print('Antares simulation duration : {}'.format(end_time - start_time))

        if returned_l.returncode != 0:
            print("WARNING: exited antares with status %d" % returned_l.returncode)
        else:
            new_output = os.listdir(self.antares_output_dir())
            assert len(old_output) + 1 == len(new_output)
            diff = list(set(new_output) - set(old_output))
            simulation_name = str(diff[0])
            StudyOutputCleaner.clean_antares_step((Path(self.antares_output_dir()) / simulation_name))

        self.simulation_name = simulation_name


    def get_antares_cmd(self):
        return [self.antares_exe_path, self.data_dir]

