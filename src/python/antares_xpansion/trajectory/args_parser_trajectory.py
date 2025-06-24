import argparse

from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters

from antares_xpansion.launcher_options_keys import LauncherOptionsKeys
from antares_xpansion.launcher_options_default_value import LauncherOptionsDefaultValues

from typing import List

from pathlib import Path
import warnings


class TrajectoryLauncherOptionsKeys:
    @staticmethod
    def step_key():
        return "step"

    @staticmethod
    def input_root_key():
        return "root"

    @staticmethod
    def input_file_key():
        return "input_file"

    @staticmethod
    def memory_key():
        return "memory"


class TrajectoryArgsParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()

        self._initialize_parser()

    def _initialize_parser(self):
        # Minimal arguments for now
        self.parser.add_argument(
            "--input-root",
            dest=TrajectoryLauncherOptionsKeys.input_root_key(),
            help="Input root, folder containing all the studies in the tree.",
            required=True,
        )
        self.parser.add_argument(
            "--input-file",
            dest=TrajectoryLauncherOptionsKeys.input_file_key(),
            help="User data input file",
            required=True,
        )
        self.parser.add_argument(
            "--step",
            dest=TrajectoryLauncherOptionsKeys.step_key(),
            choices=[
                "full",
                "input_translation",
                "problem_generation",
                "merge_master",
                "merge_weights",
                "resolution",
            ],
            help='Step to execute ("full", "input_translation", "problem_generation", "merge_master", "merge_weights", "resolution")',
            required=True,
        )
        self.parser.add_argument(
            "--memory",
            action="store_true",
            dest=TrajectoryLauncherOptionsKeys.memory_key(),
            help="Execute the problem generation in memory",
        )
        self.parser.add_argument(
            "--installDir",
            dest=LauncherOptionsKeys.installDir_key(),
            help="The directory where all binaries are located",
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
        )
        # Args for the resolution
        self.parser.add_argument(
            "-m",
            "--method",
            dest=LauncherOptionsKeys.method_key(),
            type=str,
            choices=["benders", "mergeMPS", "adequacy_criterion"],
            help="Choose the optimization method",
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
        )
        self.parser.add_argument(
            "-n",
            "--np",
            dest=LauncherOptionsKeys.n_mpi_key(),
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
            type=int,
            help="Number of MPI processes",
        )
        self.parser.add_argument(
            "--oversubscribe",
            dest=LauncherOptionsKeys.oversubscribe_key(),
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
            action="store_true",
            help="enable mpi oversubscribe option (linux only)",
        )
        self.parser.add_argument(
            "--allow-run-as-root",
            dest=LauncherOptionsKeys.allow_run_as_root_key(),
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
            action="store_true",
            help="allow-run-as-root option (linux only)",
        )

    def _warn_non_relevant_arg(self, step, arg):
        warnings.warn(f"Argument {arg} is not relevant when step is {step}, ignoring")

    def _show_args_warning(self, params):
        step = params.step
        if step not in ["resolution", "full"]:
            if params.allow_run_as_root != LauncherOptionsDefaultValues.DEFAULT_VALUE():
                self._warn_non_relevant_arg(step, "--allow-run-as-root")
            if params.oversubscribe != LauncherOptionsDefaultValues.DEFAULT_VALUE():
                self._warn_non_relevant_arg(step, "--oversubscribe")
            if params.n_mpi != LauncherOptionsDefaultValues.DEFAULT_VALUE():
                self._warn_non_relevant_arg(step, "-n / --np")
            if params.method != LauncherOptionsDefaultValues.DEFAULT_VALUE():
                self._warn_non_relevant_arg(step, "--method")
        if step != "problem_generation" and params.memory is not None:
            self._warn_non_relevant_arg(step, "--memory")

    def parse_args(self, args: List[str] = None) -> TrajectoryInputParameters:
        params = self.parser.parse_args(args)
        self._show_args_warning(params)
        self._fill_default_values(params)
        return TrajectoryInputParameters(
            step=params.step,
            input_root=Path(params.root).resolve(),
            input_file=Path(params.input_file).resolve(),
            memory=params.memory,
            install_dir=params.installDir,
            method=params.method,
            n_mpi=params.n_mpi,
            oversubscribe=params.oversubscribe,
            allow_run_as_root=params.allow_run_as_root,
        )

    def _fill_default_values(self, params):
        if params.method == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.method = LauncherOptionsDefaultValues.DEFAULT_METHOD()

        if params.n_mpi == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.n_mpi = LauncherOptionsDefaultValues.DEFAULT_NP()

        if params.oversubscribe == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.oversubscribe = LauncherOptionsDefaultValues.DEFAULT_OVERSUBSCRIBE()

        if params.allow_run_as_root == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.allow_run_as_root = (
                LauncherOptionsDefaultValues.DEFAULT_ALLOW_RUN_AS_ROOT()
            )
