import argparse
import sys
from typing import List

from antares_xpansion.__version__ import __version__, __revision__, __antares_simulator_version__
from antares_xpansion.launcher_options_default_value import LauncherOptionsDefaultValues
from antares_xpansion.launcher_options_keys import LauncherOptionsKeys
from antares_xpansion.xpansionConfig import InputParameters


class InputParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()

        self._initialize_parser()

    def _initialize_parser(self):
        self.parser.add_argument("--step",
                                 dest=LauncherOptionsKeys.step_key(),
                                 choices=["full", "antares", "problem_generation",
                                          "benders", "study_update", "sensitivity", "resume"],
                                 help='Step to execute ("full", "antares", "problem_generation", "benders", "study_update", "sensitivity", "resume")',
                                 default=LauncherOptionsDefaultValues.DEFAULT_STEP())
        self.parser.add_argument("--simulationName",
                                 dest=LauncherOptionsKeys.simulationName_key(),
                                 help="Name of the antares simulation to use. Must be present in the output directory",
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE())
        self.parser.add_argument("-i", "--dataDir",
                                 dest=LauncherOptionsKeys.dataDir_key(),
                                 help="Antares study data directory",
                                 required=True)
        self.parser.add_argument("--installDir",
                                 dest=LauncherOptionsKeys.installDir_key(),
                                 help="The directory where all binaries are located",
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE())
        self.parser.add_argument(
            "-m",
            "--method",
            dest=LauncherOptionsKeys.method_key(),
            type=str,
            choices=["benders", "mergeMPS", "adequacy_criterion"],
            help="Choose the optimization method",
            default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
        )
        self.parser.add_argument("-n", "--np",
                                 dest=LauncherOptionsKeys.n_mpi_key(),
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
                                 type=int,
                                 help='Number of MPI processes')
        self.parser.add_argument("--antares-n-cpu",
                                 dest=LauncherOptionsKeys.antares_n_cpu_key(),
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
                                 type=lambda x: (int(x) > 0) and int(x) or sys.exit(
                                     "Minimum of Antares_Simulator Thread is 1"),
                                 help='Number of Threads for Antares_Simulator')
        self.parser.add_argument("--keepMps",
                                 dest=LauncherOptionsKeys.keep_mps_key(),
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
                                 action='store_true',
                                 help='Keep .mps from lp_namer and benders steps')
        self.parser.add_argument("-v", "--version",
                                 action='version',
                                 version=__version__,
                                 help='show antares-xpansion version and exit ')
        self.parser.add_argument("--revision",
                                 action='version',
                                 version=__revision__,
                                 help='show the latest abbreviated commit hash ')
        self.parser.add_argument("--antares-version",
                                 action='version',
                                 version=__antares_simulator_version__,
                                 help='show Antares_Simulator version and exit ')
        self.parser.add_argument("--oversubscribe",
                                 dest=LauncherOptionsKeys.oversubscribe_key(),
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
                                 action='store_true',
                                 help='enable mpi oversubscribe option (linux only)')
        self.parser.add_argument("--allow-run-as-root",
                                 dest=LauncherOptionsKeys.allow_run_as_root_key(),
                                 default=LauncherOptionsDefaultValues.DEFAULT_VALUE(),
                                 action='store_true',
                                 help='allow-run-as-root option (linux only)')
        self.parser.add_argument("--memory",
                                 dest=LauncherOptionsKeys.memory_key(),
                                 default=False,
                                 action='store_true',
                                 help="Work in memory, don't write file if possible")

    def parse_args(self, args: List[str] = None) -> InputParameters:
        params = self.parser.parse_args(args)
        if params.step != "resume":
            self._fill_default_values(params)
        self._check_antares_cpu_option(params)

        my_parameters = InputParameters(
            step=params.step,
            simulation_name=params.simulationName,
            data_dir=params.dataDir,
            install_dir=params.installDir,
            method=params.method,
            n_mpi=params.n_mpi,
            antares_n_cpu=params.antares_n_cpu,
            keep_mps=params.keep_mps,
            oversubscribe=params.oversubscribe,
            allow_run_as_root=params.allow_run_as_root,
            memory=params.memory,
        )
        return my_parameters

    def _check_antares_cpu_option(self, params):

        if (params.antares_n_cpu == 1) and (params.n_mpi > 2):
            params.antares_n_cpu = params.n_mpi

    def _fill_default_values(self, params):
        if params.simulationName == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.simulationName = LauncherOptionsDefaultValues.DEFAULT_SIMULATION_NAME()

        if params.installDir == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.installDir = LauncherOptionsDefaultValues.DEFAULT_INSTALLDIR()

        if params.method == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.method = LauncherOptionsDefaultValues.DEFAULT_METHOD()

        if params.n_mpi == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.n_mpi = LauncherOptionsDefaultValues.DEFAULT_NP()

        if params.antares_n_cpu == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.antares_n_cpu = LauncherOptionsDefaultValues.DEFAULT_ANTARES_N_CPU()

        if params.keep_mps == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.keep_mps = LauncherOptionsDefaultValues.DEFAULT_KEEPMPS()

        if params.oversubscribe == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.oversubscribe = LauncherOptionsDefaultValues.DEFAULT_OVERSUBSCRIBE()

        if params.allow_run_as_root == LauncherOptionsDefaultValues.DEFAULT_VALUE():
            params.allow_run_as_root = LauncherOptionsDefaultValues.DEFAULT_ALLOW_RUN_AS_ROOT()
