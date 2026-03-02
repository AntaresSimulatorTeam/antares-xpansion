"""
Class to control the execution of Benders
"""

import glob
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

from antares_xpansion.logger import step_logger
from antares_xpansion.study_output_cleaner import StudyOutputCleaner


@dataclass
class SolversExe:
    benders: Path
    merge_mps: Path
    outer_loop: Path


class BendersDriver:
    def __init__(self, solvers_exe: SolversExe, options_file, mpiexec=None) -> None:
        self.oversubscribe = False
        self.allow_run_as_root = False
        self.benders = solvers_exe.benders
        self.merge_mps = solvers_exe.merge_mps
        self.outer_loop = solvers_exe.outer_loop
        self.mpiexec = mpiexec
        self.method = "benders"
        self.n_mpi = 1
        self.logger = step_logger(__name__, __class__.__name__)

        if options_file != "":
            self.options_file = options_file
        else:
            raise BendersDriver.BendersOptionsFileError("Invalid Options File!")

        self.MPI_N = "-n"
        self._initialise_system_specific_mpi_vars()

        # In the trajectory workflow, we want the working directory of the resolution to be the input root.
        self.use_custom_working_dir = False

    def set_custom_working_dir(self, dir: Path):
        self.use_custom_working_dir = True
        self.custom_working_dir = dir

    def launch(
            self,
            simulation_output_path,
            method,
            keep_mps=False,
            n_mpi=1,
            oversubscribe=False,
            allow_run_as_root=False,
    ):
        """
        launch the optimization of the antaresXpansion problem using the specified solver

        """
        self.logger.info("Benders")
        self.method = method
        self.n_mpi = n_mpi
        self.oversubscribe = oversubscribe
        self.allow_run_as_root = allow_run_as_root
        self.simulation_output_path = simulation_output_path
        old_cwd = os.getcwd()
        lp_path = self.get_lp_path()

        os.chdir(lp_path)
        self.logger.info(f"Current directory is now: {os.getcwd()}")

        self.set_solver()
        self.logger.info(f"Launching solver : {self.solver}")
        # delete execution logs
        self._clean_log_files()
        self.logger.info(f"Solver set to {self.solver}")
        self.logger.info(self._get_solver_cmd())
        ret = subprocess.run(
            self._get_solver_cmd(),
            shell=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        if ret.stdout:
            sys.stdout.write(ret.stdout)
        if ret.stderr:
            sys.stderr.write(ret.stderr)

        if ret.returncode != 0:
            self._try_symbolize_stacktrace(
                (ret.stdout or "") + (ret.stderr or ""),
                Path(self.solver),
            )
            self._log_symbolize_hint(lp_path, ret.returncode)
            raise BendersDriver.BendersExecutionError(
                f"ERROR: exited solver with status {ret.returncode}"
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
        # In trajectory mode, we work at the input root
        if self.use_custom_working_dir:
            lp_path = self.custom_working_dir
            return lp_path
        # Otherwise, we work in the lp folder.
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
        if self.method == "benders":
            self.solver = self.benders
        elif self.method == "adequacy_criterion":
            self.solver = self.outer_loop
        elif self.method == "mergeMPS":
            self.solver = self.merge_mps
        else:
            self.logger.error("Illegal optim method")
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
                self.logger.error("Error while deleting file : ", file_path)
        if os.path.isfile(solver_name + ".log"):
            os.remove(solver_name + ".log")

    def _get_solver_cmd(self):
        """
        returns a list consisting of the path to the required solver and its launching options
        """
        bare_solver_command = [self.solver, self.options_file]
        if self.n_mpi > 1:
            mpi_command = self.get_mpi_run_command_root()
            mpi_command.extend(bare_solver_command)
            return mpi_command
        else:
            return bare_solver_command

    def get_mpi_run_command_root(self):
        mpi_command = [self.MPI_LAUNCHER, self.MPI_N, str(self.n_mpi)]
        if sys.platform.startswith("linux"):
            if self.oversubscribe:
                mpi_command.append("--oversubscribe")
            if self.allow_run_as_root:
                mpi_command.append("--allow-run-as-root")
        return mpi_command

    def _initialise_system_specific_mpi_vars(self):
        if sys.platform.startswith("win32"):
            self.MPI_LAUNCHER = self.mpiexec
        elif sys.platform.startswith("linux"):
            self.MPI_LAUNCHER = "mpirun"
        else:
            raise (
                BendersDriver.BendersUnsupportedPlatform(
                    f"Error {sys.platform} platform is not supported \n"
                )
            )

    def _log_symbolize_hint(self, work_dir: Path, returncode: int):
        if not sys.platform.startswith("linux"):
            return
        self.logger.error(
            "Solver exited with status %s. Core dumps are disabled; run under gdb for a symbolized stack trace.",
            returncode,
        )
        self.logger.error(
            "Example: cd %s && gdb --args %s %s",
            work_dir,
            self.solver,
            self.options_file,
        )

    def _try_symbolize_stacktrace(self, output: str, binary_path: Path):
        if not sys.platform.startswith("linux"):
            return
        if not output:
            return
        if shutil.which("addr2line") is None:
            self.logger.error("addr2line not found; install binutils to symbolize stack traces.")
            return

        frames = self._extract_stacktrace_frames(output, binary_path)
        if not frames:
            return

        self.logger.error("Symbolized stack trace (addr2line):")
        for entry in frames:
            symbol = self._addr2line_symbol(binary_path, entry["offset"])
            self.logger.error("%s [%s] %s", entry["prefix"], entry["frame"], symbol)

    def _extract_stacktrace_frames(self, output: str, binary_path: Path) -> List[Dict[str, str]]:
        binary_name = binary_path.name
        frame_regex = re.compile(
            r"^\[(?P<prefix>[^\]]+)\]\s+\[\s*(?P<frame>\d+)\]\s+"
            r"(?P<binary>\S+)\(\+0x(?P<offset>[0-9a-fA-F]+)\)\[0x[0-9a-fA-F]+\]"
        )
        frames: List[Dict[str, str]] = []
        for line in output.splitlines():
            match = frame_regex.search(line)
            if not match:
                continue
            binary = Path(match.group("binary")).name
            if binary != binary_name:
                continue
            frames.append(
                {
                    "prefix": f"[{match.group('prefix')}]",
                    "frame": match.group("frame"),
                    "offset": f"0x{match.group('offset')}",
                }
            )
        return frames

    def _addr2line_symbol(self, binary_path: Path, offset: str) -> str:
        try:
            # First try with -p (pretty print) and -s (basename only) for better output
            result = subprocess.run(
                [
                    "addr2line",
                    "-e",
                    str(binary_path),
                    "-f",  # Show function names
                    "-C",  # Demangle C++ names
                    "-i",  # Show inlined functions
                    "-p",  # Pretty print (more readable)
                    "-s",  # Strip directory names from file paths
                    offset,
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            return result.stdout.strip()

        except Exception as exc:
            return f"addr2line failed: {exc}"

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

    class BendersOptionsFileError(Exception):
        pass
