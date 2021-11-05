"""
    Class to control the execution of Benders
"""

from dataclasses import dataclass
import glob
import os
from pathlib import Path
import subprocess
import sys



from antares_xpansion.study_output_cleaner import StudyOutputCleaner

import functools

print = functools.partial(print, flush=True)

class BendersDriverException :
    class BaseException(Exception):
        pass
    class BendersOutputPathError(BaseException):
        pass
    class BendersUnsupportedPlatform(BaseException):
        pass
    class BendersLp_PathError(BaseException):
        pass
@dataclass
class BendersDriverData:
    install_dir        : Path
    BENDERS_MPI        : str
    method             : str
    MERGE_MPS          : str
    BENDERS_SEQUENTIAL : str
    n_mpi              : int
    keep_mps           : bool
class BendersDriver:

    def __init__(self, benders_driver_data : BendersDriverData) -> None:

        self.install_dir         = benders_driver_data.install_dir
        self.BENDERS_MPI         = benders_driver_data.BENDERS_MPI
        self.method              = benders_driver_data.method
        self.MERGE_MPS           = benders_driver_data.MERGE_MPS
        self.BENDERS_SEQUENTIAL  = benders_driver_data.BENDERS_SEQUENTIAL
        self.n_mpi               = benders_driver_data.n_mpi
        self.keep_mps            = benders_driver_data.keep_mps

        self.OPTIONS_TXT         = 'options.txt'
        self._initialise_system_specific_mpi_vars()

    def launch(self, simulation_output_path):
        """
            launch the optimization of the antaresXpansion problem using the specified solver

            XpansionConfig.BENDERS_SEQUENTIAL]
        """
        print ("-- Benders")
        self.simulation_output_path = simulation_output_path
        old_cwd = os.getcwd()
        lp_path =  self.get_lp_path()
        os.chdir(lp_path)
        print('Current directory is now : ', os.getcwd())

        solver = self.get_solver()

        # delete execution logs
        self.clean_log_files(solver)
        returned_l = subprocess.run(self.get_solver_cmd(solver), shell=False,
                                    stdout=sys.stdout,
                                    stderr=sys.stderr)
        if returned_l.returncode != 0:
            print("ERROR: exited solver with status %d" % returned_l.returncode)
            sys.exit(1)
        elif not self.keep_mps:
            StudyOutputCleaner.clean_benders_step(self.simulation_output_path)
        os.chdir(old_cwd)

    def set_simulation_output_path(self, simulation_output_path : Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else : 
            raise BendersDriverException.BendersOutputPathError(f"Benders Error: {simulation_output_path} not found ")

    def get_simulation_output_path(self):
        return self._simulation_output_path

    def get_lp_path(self):
        lp_path =  Path(os.path.normpath(os.path.join(self.simulation_output_path, 'lp')))
        if(lp_path.is_dir()):
            return lp_path
        else :
            raise BendersDriverException.BendersLp_PathError(f"Error in lp path: {lp_path} not found")


    def get_solver(self):
        solver = None
        if self.method == "mpibenders":
            solver = self.BENDERS_MPI
        elif self.method == "mergeMPS":
            solver = self.MERGE_MPS
        elif self.method == "sequential":
            solver = self.BENDERS_SEQUENTIAL
        elif self.method == "both":
            print('method "both" is not handled yet')
            sys.exit(1)
        else:
            print("Illegal optim method")
            sys.exit(1) 

        return solver

    def clean_log_files(self, solver):
        logfile_list = glob.glob('./' + solver + 'Log*')
        for file_path in logfile_list:
            try:
                os.remove(file_path)
            except OSError:
                print("Error while deleting file : ", file_path)
        if os.path.isfile(solver + '.log'):
            os.remove(solver + '.log')    

    def get_solver_cmd(self, solver):
        """
            returns a list consisting of the path to the required solver and its launching options
        """
        assert solver in [self.MERGE_MPS,
                          self.BENDERS_MPI,
                          self.BENDERS_SEQUENTIAL]
        if solver == self.BENDERS_MPI:
            return [self.MPI_LAUNCHER, self.MPI_N, str(self.n_mpi), self._exe_path(solver),
                    self.OPTIONS_TXT]
        else:
            return [self._exe_path(solver), self.OPTIONS_TXT]

    def _exe_path(self, exe):
        """
            prefixes the input exe with the install directory containing the binaries

            :param exe: executable name

            :return: path to specified executable
        """
        return os.path.normpath(os.path.join(self.install_dir, exe))

    
    def _initialise_system_specific_mpi_vars(self):
        if sys.platform.startswith("win32"):
            self.MPI_LAUNCHER = "mpiexec"
            self.MPI_N = "-n"
        elif sys.platform.startswith("linux"):
            self.MPI_LAUNCHER = "mpirun"
            self.MPI_N = "-np"
        else:
            raise(BendersDriverException.BendersUnsupportedPlatform(f"Error {sys.platform} platform is not supported \n"))
    
    simulation_output_path = property(get_simulation_output_path, set_simulation_output_path)