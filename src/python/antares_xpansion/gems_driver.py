"""
    Class to control the execution of the gems step (antares-problem-generator + benders)
"""

import os
import shutil
import subprocess
from pathlib import Path
from typing import List

from antares_xpansion.logger import step_logger


class GemsDriver:
    """
        Initialize Gems Driver with given binary paths
    """

    def __init__(
        self,
        antares_problem_generator_exe: Path,
        benders_driver,
        config_loader,
    ) -> None:
        self.antares_problem_generator_exe = Path(antares_problem_generator_exe)
        self.benders_driver = benders_driver
        self.config_loader = config_loader
        self.data_dir = ""
        self.output = "output"
        self.logger = step_logger(__name__, __class__.__name__)

    def launch(
        self,
        study_path: Path,
        method: str,
        keep_mps: bool,
        n_mpi: int,
        oversubscribe: bool = False,
        allow_run_as_root: bool = False,
    ) -> bool:
        """
        Launch the gems step: antares-problem-generator + benders
        """
        self.data_dir = study_path

        self._run_antares_problem_generator()
        self._set_simulation_name()
        self.config_loader._xpansion_simulation_name = self.simulation_output_path()
        self._create_lp_directory()
        self.config_loader.benders_pre_actions()
        self._run_benders(
            method, keep_mps, n_mpi, oversubscribe, allow_run_as_root
        )
        return True

    def _run_antares_problem_generator(self):
        self.logger.info("Launching antares-problem-generator")

        cmd = self._get_antares_problem_generator_cmd()
        self.logger.info(cmd)

        returned_l = subprocess.run(
            cmd,
            shell=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if returned_l.returncode != 0:
            raise GemsDriver.GemsExecutionError(
                f"Error: exited antares-problem-generator with status {returned_l.returncode}"
            )

    def _get_antares_problem_generator_cmd(self) -> List[str]:
        cmd = [
            str(self.antares_problem_generator_exe),
            str(self.data_dir),
        ]
        return cmd

    def antares_output_dir(self):
        """
            returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir, self.output))

    def _set_simulation_name(self):
        self.simulation_name = ""
        list_of_dirs_filter = filter(
            lambda x: os.path.isdir(os.path.join(self.antares_output_dir(), x)),
            os.listdir(self.antares_output_dir()),
        )
        list_of_dirs = sorted(
            list_of_dirs_filter,
            key=lambda x: os.path.getmtime(
                os.path.join(self.antares_output_dir(), x)
            ),
        )
        if list_of_dirs:
            self.simulation_name = list_of_dirs[-1]

    def _create_lp_directory(self):
        output_path = self.simulation_output_path()
        lp_path = output_path / "lp"
        if lp_path.exists():
            shutil.rmtree(lp_path)
        os.makedirs(lp_path)
        self._copy_mps_files(output_path, lp_path)

    def _copy_mps_files(self, output_path, lp_path):
        for f in os.listdir(output_path):
            if f.endswith(".mps") or f == "structure.txt":
                src = output_path / f
                dst = lp_path / f
                shutil.copy2(src, dst)

    def simulation_output_path(self) -> Path:
        return Path(self.antares_output_dir()) / self.simulation_name

    def _run_benders(
        self,
        method: str,
        keep_mps: bool,
        n_mpi: int,
        oversubscribe: bool,
        allow_run_as_root: bool,
    ):
        self.logger.info("Running benders")

        output_path = self.simulation_output_path()
        lp_path = output_path / "lp"
        self.benders_driver.set_custom_working_dir(lp_path)
        self.benders_driver.set_simulation_output_path(output_path)

        self.benders_driver.launch(
            simulation_output_path=output_path,
            method=method,
            keep_mps=keep_mps,
            n_mpi=n_mpi,
            oversubscribe=oversubscribe,
            allow_run_as_root=allow_run_as_root,
        )

    class GemsExecutionError(Exception):
        pass
