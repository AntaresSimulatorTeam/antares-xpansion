import argparse
import warnings
from pathlib import Path
from typing import List

from antares_xpansion.launcher_options_default_value import LauncherOptionsDefaultValues
from antares_xpansion.launcher_options_keys import LauncherOptionsKeys
from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters


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

    @staticmethod
    def problems_format_key():
        return "problems_format"

    @staticmethod
    def solver_key():
        return "solver"

    @staticmethod
    def cache_problems_key():
        return "cache_problems"


class TrajectoryLauncherOptionsDefaultValues:
    @staticmethod
    def problems_format_default():
        return "saved"

    @staticmethod
    def solver_default():
        return "Xpress"

    @staticmethod
    def default_input_file_name():
        return "input-trajectory.yaml"


class TrajectoryArgsParser:
    class XpansionTrajectoryInvalidArguments(Exception):
        pass

    def __init__(self):
        self.parser = argparse.ArgumentParser()

        self._initialize_parser()

    def _initialize_parser(self):
        # Minimal arguments for now
        self.parser.add_argument(
            "-i",
            "--dataDir",
            dest=TrajectoryLauncherOptionsKeys.input_root_key(),
            help="Folder containing all the studies in the tree. Defaults to current directory.",
            required=False,
            default=None,
        )
        self.parser.add_argument(
            "--input-file",
            dest=TrajectoryLauncherOptionsKeys.input_file_key(),
            help="User data input file. If not specified, looks for 'input-trajectory.yaml' in the input-root folder.",
            required=False,
            default=None,
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
        # When manipulating problem files : under which format are they written ?
        self.parser.add_argument(
            "--problems-format",
            dest=TrajectoryLauncherOptionsKeys.problems_format_key(),
            type=str,
            choices=["saved", "mps"],
            help="Format under which problem files should be read and written - 'saved' default only compatible with solver 'xpress'.",
            default=TrajectoryLauncherOptionsDefaultValues.problems_format_default(),
        )
        # What type of solver should we use to perform problem merging and resolution
        # (does not apply to problem generation, where the solver used is given in 'user/expansion/settings.ini')
        # Choices are hardcoded but should also be matched with 'AVAILABLE_SOLVERS' later ...
        self.parser.add_argument(
            "--solver",
            dest=TrajectoryLauncherOptionsKeys.solver_key(),
            type=str,
            choices=["Xpress", "Cbc", "Coin"],
            help="Name of the solver used to perform problem merging and resolution - does not apply to problem generation.",
            default=TrajectoryLauncherOptionsDefaultValues.solver_default(),
        )
        self.parser.add_argument(
            "--cache_problems",
            dest=TrajectoryLauncherOptionsKeys.cache_problems_key(),
            default=False,
            action="store_true",
            help="Cache problems on disk during benders",
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

    def _assert_args_compatibility(self, params):
        """Checks that the given args are compatible with each other"""
        if params.problems_format == "saved" and params.solver != "Xpress":
            raise self.XpansionTrajectoryInvalidArguments(
                "Argument '--problems-format saved' is only compatible with '--solver Xpress'"
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
        if step != "problem_generation" and params.memory:
            self._warn_non_relevant_arg(step, "--memory")

    def parse_args(self, args: List[str] = None) -> TrajectoryInputParameters:
        params = self.parser.parse_args(args)
        self._assert_args_compatibility(params)
        self._show_args_warning(params)
        self._fill_default_values(params)

        # Handle input_root: use current directory if not specified
        if params.root is None:
            input_root = Path.cwd()
        else:
            input_root = Path(params.root).resolve()

        # Handle input_file: use default name if not specified
        if params.input_file is None:
            input_file = input_root / TrajectoryLauncherOptionsDefaultValues.default_input_file_name()
        else:
            input_file = Path(params.input_file).resolve()

        # Validate that the input file exists
        if not input_file.exists():
            raise self.XpansionTrajectoryInvalidArguments(
                f"Input file not found: {input_file}\n"
                f"Expected file '{TrajectoryLauncherOptionsDefaultValues.default_input_file_name()}' "
                f"in input-root directory: {input_root}"
            )

        return TrajectoryInputParameters(
            step=params.step,
            input_root=input_root,
            input_file=input_file,
            memory=params.memory,
            install_dir=params.installDir,
            problems_format=params.problems_format,
            solver=params.solver,
            cache_problems=params.cache_problems,
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
