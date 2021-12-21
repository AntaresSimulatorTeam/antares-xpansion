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
from antares_xpansion.flushed_print import flushed_print


class BendersDriver:
    def __init__(self, benders_mpi, benders_sequential, merge_mps) -> None:

        self.oversubscribe = False
        self.benders_mpi = benders_mpi
        self.merge_mps = merge_mps
        self.benders_sequential = benders_sequential

        self.OPTIONS_TXT = "options.txt"
        self._initialise_system_specific_mpi_vars()

    def launch(self, simulation_output_path, method, keep_mps=False, n_mpi=1, oversubscribe=False):
        """
        launch the optimization of the antaresXpansion problem using the specified solver

        """
        flushed_print("-- Benders")
        self.method = method
        self.n_mpi = n_mpi
        self.oversubscribe = oversubscribe
        self.simulation_output_path = simulation_output_path
        old_cwd = os.getcwd()
        lp_path = self.get_lp_path()
        os.chdir(lp_path)
        flushed_print("Current directory is now: ", os.getcwd())

        self.set_solver()

        # delete execution logs
        self._clean_log_files()
        returned_l = subprocess.run(
            self._get_solver_cmd(), shell=False, stdout=sys.stdout, stderr=sys.stderr
        )
        if returned_l.returncode != 0:
            raise BendersDriver.BendersExecutionError(
                "ERROR: exited solver with status %d" % returned_l.returncode
            )
        elif not keep_mps:
            StudyOutputCleaner.clean_benders_step(self.simulation_output_path)
        os.chdir(old_cwd)

    def set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else:
            raise BendersDriver.BendersOutputPathError(
                f"Benders Error: {simulation_output_path} not found "
            )

    def get_simulation_output_path(self):
        return self._simulation_output_path

    def get_lp_path(self):
        lp_path = Path(
            os.path.normpath(os.path.join(self.simulation_output_path, "lp"))
        )
        if lp_path.is_dir():
            return lp_path
        else:
            raise BendersDriver.BendersLpPathError(
                f"Error in lp path: {lp_path} not found"
            )

    def set_solver(self):
        if self.method == "mpibenders":
            self.solver = self.benders_mpi
        elif self.method == "mergeMPS":
            self.solver = self.merge_mps
        elif self.method == "sequential":
            self.solver = self.benders_sequential
        else:
            flushed_print("Illegal optim method")
            raise BendersDriver.BendersSolverError(
                f" {self.method} method is unavailable !"
            )

    def _clean_log_files(self):
        solver_name = Path(self.solver).name
        logfile_list = glob.glob("./" + solver_name + "Log*")
        for file_path in logfile_list:
            try:
                os.remove(file_path)
            except OSError:
                flushed_print("Error while deleting file : ", file_path)
        if os.path.isfile(solver_name + ".log"):
            os.remove(solver_name + ".log")

    def _get_solver_cmd(self):
        """
        returns a list consisting of the path to the required solver and its launching options
        """
        bare_solver_command = [self.solver, self.OPTIONS_TXT]
        if self.solver == self.benders_mpi:
            mpi_command = self._get_mpi_run_command_root()
            mpi_command.extend(bare_solver_command)
            return mpi_command
        else:
            return bare_solver_command

    def _get_mpi_run_command_root(self):

        mpi_command = [self.MPI_LAUNCHER, self.MPI_N, str(self.n_mpi)]
        if sys.platform.startswith("linux") and self.oversubscribe:
            mpi_command.append("--oversubscribe")
        return mpi_command

    def _initialise_system_specific_mpi_vars(self):
        if sys.platform.startswith("win32"):
            self.MPI_LAUNCHER = "mpiexec"
            self.MPI_N = "-n"
        elif sys.platform.startswith("linux"):
            self.MPI_LAUNCHER = "mpirun"
            self.MPI_N = "-np"
        else:
            raise (
                BendersDriver.BendersUnsupportedPlatform(
                    f"Error {sys.platform} platform is not supported \n"
                )
            )

    simulation_output_path = property(
        get_simulation_output_path, set_simulation_output_path
    )

    class BendersOutputPathError(Exception):
        pass

    class BendersUnsupportedPlatform(Exception):
        pass

    class BendersLpPathError(Exception):
        pass

    class BendersSolverError(Exception):
        pass

    class BendersExecutionError(Exception):
        pass
