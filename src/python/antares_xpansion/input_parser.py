import argparse
from typing import List

import sys

from antares_xpansion.xpansionConfig import InputParameters
from antares_xpansion.__version__ import __version__, __antares_simulator_version__
from antares_xpansion.flushed_print import flushed_print, INFO_MSG


class InputParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()
        self.DEFAULT_STEP = "full"
        self.DEFAULT_SIMULATION_NAME = "last"
        self.DEFAULT_INSTALLDIR = None
        self.DEFAULT_METHOD = "sequential"
        self.DEFAULT_NP = 2
        self.DEFAULT_ANTARES_N_CPU = 1
        self.DEFAULT_KEEPMPS = False
        self.DEFAULT_OVERSUBSCRIBE = False
        self.DEFAULT_ALLOW_RUN_AS_ROOT = False
        self.DEFAULT_RESUME = False
        self._initialize_parser()

    def _initialize_parser(self):
        self.parser.add_argument("--step",
                                 dest="step",
                                 choices=["full", "antares", "problem_generation",
                                          "benders", "study_update", "sensitivity", "resume"],
                                 help='Step to execute ("full", "antares", "problem_generation", "benders", "study_update", "sensitivity", "resume")',
                                 default=self.DEFAULT_STEP)
        self.parser.add_argument("--simulationName",
                                 dest="simulationName",
                                 help="Name of the antares simulation to use. Must be present in the output directory",
                                 default=self.DEFAULT_SIMULATION_NAME)
        self.parser.add_argument("-i", "--dataDir",
                                 dest="dataDir",
                                 help="Antares study data directory",
                                 required=True)
        self.parser.add_argument("--installDir",
                                 dest="installDir",
                                 help="The directory where all binaries are located",
                                 default=self.DEFAULT_INSTALLDIR)
        self.parser.add_argument("-m", "--method",
                                 dest="method",
                                 type=str,
                                 choices=["mpibenders",
                                          "mergeMPS", "sequential"],
                                 help="Choose the optimization method",
                                 default=self.DEFAULT_METHOD)
        self.parser.add_argument("-n", "--np",
                                 dest="n_mpi",
                                 default=self.DEFAULT_NP,
                                 type=lambda x: (int(x) > 1) and int(x) or sys.exit(
                                     "Minimum of MPI processes is 2"),
                                 help='Number of MPI processes')
        self.parser.add_argument("--antares-n-cpu",
                                 dest="antares_n_cpu",
                                 default=self.DEFAULT_ANTARES_N_CPU,
                                 type=lambda x: (int(x) > 0) and int(x) or sys.exit(
                                     "Minimum of Antares_Simulator Thread is 1"),
                                 help='Number of Threads for Antares_Simulator')
        self.parser.add_argument("--keepMps",
                                 dest="keep_mps",
                                 default=self.DEFAULT_KEEPMPS,
                                 action='store_true',
                                 help='Keep .mps from lp_namer and benders steps')
        self.parser.add_argument("-v", "--version",
                                 action='version',
                                 version=__version__,
                                 help='show antares-xpansion version and exit ')
        self.parser.add_argument("--antares-version",
                                 action='version',
                                 version=__antares_simulator_version__,
                                 help='show Antares_Simulator version and exit ')
        self.parser.add_argument("--oversubscribe",
                                 dest="oversubscribe",
                                 default=self.DEFAULT_OVERSUBSCRIBE,
                                 action='store_true',
                                 help='enable mpi oversubscribe option (linux only)')
        self.parser.add_argument("--allow-run-as-root",
                                 dest="allow_run_as_root",
                                 default=self.DEFAULT_ALLOW_RUN_AS_ROOT,
                                 action='store_true',
                                 help='allow-run-as-root option (linux only)')
        self.parser.add_argument("--resume",
                                 dest="resume",
                                 default=self.DEFAULT_RESUME,
                                 action='store_true',
                                 help='restart last study')

    def parse_args(self, args: List[str] = None) -> InputParameters:
        params = self.parser.parse_args(args)
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
            allow_run_as_root=params.allow_run_as_root
        )
        return my_parameters

    def _check_antares_cpu_option(self, params):

        if (params.antares_n_cpu == 1) and (params.n_mpi > 2):
            params.antares_n_cpu = params.n_mpi
