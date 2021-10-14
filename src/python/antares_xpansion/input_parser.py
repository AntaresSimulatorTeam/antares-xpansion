import argparse
from typing import List

import sys

from antares_xpansion.xpansionConfig import InputParameters


class InputParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()
        self._initialize_parser()

    def _initialize_parser(self):
        self.parser.add_argument("--step",
                                 dest="step",
                                 choices=["full", "antares", "getnames", "lp", "optim", "update"],
                                 help='Step to execute ("full", "antares", "getnames", "lp", "optim", "update")',
                                 default="full")
        self.parser.add_argument("--simulationName",
                                 dest="simulationName",
                                 help="Name of the antares simulation to use. Must be present in the output directory",
                                 default="last")
        self.parser.add_argument("-i", "--dataDir",
                                 dest="dataDir",
                                 help="Antares study data directory",
                                 required=True)
        self.parser.add_argument("--installDir",
                                 dest="installDir",
                                 help="The directory where all binaries are located",
                                 default=None)
        self.parser.add_argument("-m", "--method",
                                 dest="method",
                                 type=str,
                                 choices=["mpibenders", "mergeMPS", "sequential"],
                                 help="Choose the optimization method",
                                 default="sequential")
        self.parser.add_argument("-n", "--np",
                                 dest="n_mpi",
                                 default=4,
                                 type=lambda x: (int(x) > 1) and int(x) or sys.exit("Minimum of MPI processes is 1"),
                                 help='Number of MPI processes')
        self.parser.add_argument("--keepMps",
                                 dest="keep_mps",
                                 default=False,
                                 action='store_true',
                                 help='Keep .mps from lp_namer and benders steps')

    def parse_args(self, args: List[str] = None) -> InputParameters:
        params = self.parser.parse_args(args)
        my_parameters = InputParameters(
            step=params.step,
            simulationName=params.simulationName,
            dataDir=params.dataDir,
            installDir=params.installDir,
            method=params.method,
            n_mpi=params.n_mpi,
            keep_mps=params.keep_mps,
        )
        return my_parameters
