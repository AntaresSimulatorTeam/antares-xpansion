"""
    Class to control the execution of Benders
"""

import glob
import os
import subprocess
import sys



from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.config_loader import ConfigLoader


class BendersDriver:
    def __init__(self, config_loader : ConfigLoader) -> None:
        self.config_loader = config_loader
        self.config = self.config_loader.config
        self.options = self.config_loader.options

    def launch(self):
        self.launch_optimization()

    def launch_optimization(self):
        """
            launch the optimization of the antaresXpansion problem using the specified solver

            XpansionConfig.BENDERS_SEQUENTIAL]
        """
        print ("-- Benders")
        self.config_loader.set_options_for_benders_solver()
        lp_path = self.config_loader.simulation_lp_path()

        old_cwd = os.getcwd()
        os.chdir(lp_path)
        print('Current directory is now : ', os.getcwd())

        solver = None
        if self.config.method == "mpibenders":
            solver = self.config.BENDERS_MPI
        elif self.config.method == "mergeMPS":
            solver = self.config.MERGE_MPS
        elif self.config.method == "sequential":
            solver = self.config.BENDERS_SEQUENTIAL
        elif self.config.method == "both":
            print('method "both" is not handled yet')
            sys.exit(1)
        else:
            print("Illegal optim method")
            sys.exit(1)

        # delete execution logs
        logfile_list = glob.glob('./' + solver + 'Log*')
        for file_path in logfile_list:
            try:
                os.remove(file_path)
            except OSError:
                print("Error while deleting file : ", file_path)
        if os.path.isfile(solver + '.log'):
            os.remove(solver + '.log')

        returned_l = subprocess.run(self.get_solver_cmd(solver), shell=False,
                                    stdout=sys.stdout,
                                    stderr=sys.stderr)
        if returned_l.returncode != 0:
            print("ERROR: exited solver with status %d" % returned_l.returncode)
            sys.exit(1)
        elif not self.config.keep_mps:
            StudyOutputCleaner.clean_benders_step(self.config_loader.simulation_output_path())
        os.chdir(old_cwd)

    def get_solver_cmd(self, solver):
        """
            returns a list consisting of the path to the required solver and its launching options
        """
        assert solver in [self.config.MERGE_MPS,
                          self.config.BENDERS_MPI,
                          self.config.BENDERS_SEQUENTIAL]
        if solver == self.config.BENDERS_MPI:
            return [self.config.MPI_LAUNCHER, self.config.MPI_N, str(self.config.n_mpi), self.config_loader.exe_path(solver),
                    self.config.OPTIONS_TXT]
        else:
            return [self.config_loader.exe_path(solver), self.config.OPTIONS_TXT]
