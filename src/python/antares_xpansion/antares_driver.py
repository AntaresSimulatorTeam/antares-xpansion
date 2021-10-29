"""
    Class to control the execution of the antares step
"""

import os
import subprocess
from datetime import datetime
from pathlib import Path


from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.general_data_processor import GeneralDataProcessor
import functools

print = functools.partial(print, flush=True)


class AntaresDriver:
    """
        Initialize Antares Driver with given binary path to Antares solver
    """
    def __init__(self, antares_exe_path: Path) -> None:
        
        self.antares_exe_path = antares_exe_path
        #antares study dir is just before launch 
        self.data_dir =  "" 

        self.settings = 'settings'
        self.general_data_ini = 'generaldata.ini'
        self.output = 'output'
        self.ANTARES_N_CPU_OPTION = "--force-parallel"
        self.antares_n_cpu = 1 # default
        self.is_accurate = False
        
    def launch_accurate_mode(self, antares_study_path, antares_n_cpu : int):
        self.is_accurate = True
        self._set_antares_n_cpu(antares_n_cpu)
        self._launch(antares_study_path)  

    def launch_fast_mode(self, antares_study_path, antares_n_cpu : int):
        self.is_accurate = False
        self._set_antares_n_cpu(antares_n_cpu)
        self._launch(antares_study_path)  

    def _set_antares_n_cpu(self, antares_n_cpu : int):
        if antares_n_cpu >= 1 :
            self.antares_n_cpu = antares_n_cpu 
        else :
            print(f"WARNING! value antares_n_cpu= {antares_n_cpu} is not accepted, default value will be used.")


    def _launch(self, antares_study_path):
        self._clear_old_log()
        self.data_dir = antares_study_path      
        self._update_general_data_ini()                                  
        self.launch_antares()

    def _update_general_data_ini(self):
        settings_dir = os.path.normpath(os.path.join(self.data_dir, self.settings))
        gen_data_proc = GeneralDataProcessor(settings_dir, self.is_accurate)
        gen_data_proc.change_general_data_file_to_configure_antares_execution() 

    def _clear_old_log(self):
        if os.path.isfile(self.antares_exe_path + '.log'):
            os.remove(self.antares_exe_path + '.log')


    def antares_output_dir(self):
        """
            returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir, self.output))                                             



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
        return [self.antares_exe_path, self.data_dir, self.ANTARES_N_CPU_OPTION, str(self.antares_n_cpu)]
        

