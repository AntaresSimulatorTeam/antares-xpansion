"""
    Class to control the Problem Generation
"""

import shutil
import os
import subprocess
import sys
from datetime import datetime
from dataclasses import dataclass
from pathlib import Path

from antares_xpansion.xpansion_utils import read_and_write_mps
from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from antares_xpansion.xpansion_study_reader import XpansionStudyReader

import functools

print = functools.partial(print, flush=True)

class ProblemGeneratorException:
    class BaseException(Exception):
        pass
    class AreaFileException(BaseException):
        pass
    class IntercoFilesException(BaseException):
        pass
    class OutputPathError(BaseException):
        pass
    class LPNamerExeError(BaseException):
        pass
@dataclass
class ProblemGeneratorData:
    LP_NAMER : str
    keep_mps : bool
    additional_constraints : str
    weights_file_path : Path
    weight_file_name : str
    install_dir : Path
class ProblemGeneratorDriver:
    def __init__(self, problem_generator_data : ProblemGeneratorData) -> None:

        self.LP_NAMER = problem_generator_data.LP_NAMER
        self.keep_mps = problem_generator_data.keep_mps
        self.additional_constraints = problem_generator_data.additional_constraints
        self.weights_file_path = problem_generator_data.weights_file_path
        self.weight_file_name = problem_generator_data.weight_file_name
        self.install_dir = problem_generator_data.install_dir
        self.MPS_TXT = "mps.txt"
        self.is_relaxed = False


    def launch(self, output_path : Path, is_relaxed: bool):
        """
            problem generation step : getnames + lp_namer
        """
        self._clear_old_log()
        if output_path.exists():
            print("-- Problem Generation")
            self.output_path = output_path

            self._get_names()

            self.lp_path = os.path.normpath(os.path.join(self.output_path, 'lp'))
            self.is_relaxed = is_relaxed
            self._lp_step()
        else :
            raise ProblemGeneratorException.OutputPathError(f"{output_path} not found")

    def _clear_old_log(self):
        if (os.path.isfile(self._exe_path(self.LP_NAMER) + '.log')):
            os.remove(self._exe_path(self.LP_NAMER) + '.log')

    def _exe_path(self, exe):
        """
            prefixes the input exe with the install directory containing the binaries

            :param exe: executable name

            :return: path to specified executable
        """
        return os.path.normpath(os.path.join(self.install_dir, exe))
   
    def _get_names(self):
        """
            produces a .txt file describing the weekly problems:
            each line of the file contains :
             - mps file name
             - variables file name
             - constraints file name

            produces a file named with xpansionConfig.MPS_TXT
        """

        
        mps_txt = read_and_write_mps(self.output_path)
        with open(os.path.normpath(os.path.join(self.output_path, self.MPS_TXT)), 'w') as file_l:
            for line in mps_txt.items():
                file_l.write(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + '\n')

        self._check_and_copy_area_file()
        self._check_and_copy_interco_file()


    def _check_and_copy_area_file(self):
        self._check_and_copy_txt_file("area", ProblemGeneratorException.AreaFileException)

    def _check_and_copy_interco_file(self):
        self._check_and_copy_txt_file("interco", ProblemGeneratorException.IntercoFilesException)

    def _check_and_copy_txt_file(self, prefix, exception_to_raise: ProblemGeneratorException.BaseException):
        self._check_and_copy_file(prefix, "txt", exception_to_raise)

    def _check_and_copy_file(self, prefix, extension, exception_to_raise : ProblemGeneratorException.BaseException):
        glob_path = Path(self.output_path)
        files = [str(pp) for pp in glob_path.glob(prefix+"*"+extension)]
        if len(files) == 0 :
            raise exception_to_raise("No %s*.txt file found"%prefix)

        elif len(files) > 1 :
            raise exception_to_raise("More than one %s*.txt file found"%prefix)

        shutil.copy(files[0], os.path.normpath(os.path.join(self.output_path, prefix+'.'+extension)))

    def _lp_step(self):
        """
            copies area and interco files and launches the lp_namer

            produces a file named with xpansionConfig.MPS_TXT
        """

        if os.path.isdir(self.lp_path):
            shutil.rmtree(self.lp_path)
        os.makedirs(self.lp_path)

        
        if self.weight_file_name:
            weight_list = XpansionStudyReader.get_years_weight_from_file(self.weights_file_path)
            YearlyWeightWriter(Path(self.output_path)).create_weight_file(weight_list, self.weight_file_name)

        with open(self.get_lp_namer_log_filename(), 'w') as output_file:

            start_time = datetime.now()
            returned_l = subprocess.run(self.get_lp_namer_command(), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)

            end_time = datetime.now()
            print('Post antares step duration: {}'.format(end_time - start_time))

            if returned_l.returncode != 0:
                print("ERROR: exited lpnamer with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.keep_mps:
                StudyOutputCleaner.clean_lpnamer_step(Path(self.output_path))


    def get_lp_namer_log_filename(self):
        return os.path.join(self.lp_path, self.LP_NAMER + '.log')

    def get_lp_namer_command(self):
        
        is_relaxed = 'relaxed' if self.is_relaxed else 'integer'
        lp_namer_exe = Path(self._exe_path(self.LP_NAMER))
        if not isinstance(self.output_path, Path):
            raise ProblemGeneratorException.OutputPathError(f"Error {self.output_path} is not a valid Path")
        elif not self.output_path.exists() :
            raise ProblemGeneratorException.OutputPathError(f"Error {self.output_path} not Found")
        elif  not lp_namer_exe.is_file():
            raise ProblemGeneratorException.LPNamerExeError(f"LP namer exe : {lp_namer_exe} not found")
        
        return [lp_namer_exe, "-o", self.output_path, "-f", is_relaxed, "-e",
                self.additional_constraints]


