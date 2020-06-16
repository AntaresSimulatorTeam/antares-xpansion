"""
    parameters for an AntaresXpansion session
"""

import argparse

class XpansionConfig(object):
    """
        Class defininf the parameters for an AntaresXpansion session
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods

    def __init__(self):
        self.MPI_LAUNCHER = "mpiexec"
        self.MPI_N = "-n"
        self.ANTARES = 'antares-7.0-solver'
        self.SETTINGS = 'settings'
        self.USER = 'user'
        self.EXPANSION = 'expansion'
        self.CAPADIR = 'capa'
        self.GENERAL_DATA_INI = 'generaldata.ini'
        self.NB_YEARS = 'nbyears'
        self.SETTINGS_INI = 'settings.ini'
        self.CANDIDATES_INI = 'candidates.ini'
        self.UC_TYPE = 'uc_type'
        self.EXPANSION_ACCURATE = 'expansion_accurate'
        self.EXPANSION_FAST = 'expansion_fast'
        self.OPTIMIZATION = 'optimization'
        self.EXPORT_STRUCTURE = 'include-exportstructure'
        self.EXPORT_MPS = 'include-exportmps'
        self.TRACE = 'include-trace'
        self.USE_XPRS = 'include-usexprs'
        self.INBASIS = 'include-inbasis'
        self.OUTBASIS = 'include-outbasis'

        self.OUTPUT = 'output'
        self.OPTIONS_TXT = 'options.txt'
        self.MERGE_MPS = "merge_mps"
        self.MPS_TXT = "mps.txt"
        self.BENDERS_MPI = "bendersmpi"
        self.BENDERS_SEQUENTIAL = "benderssequential"
        self.LP_NAMER = "lp_namer"

        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--step", choices=["lp", "optim", "full", "antares", "getnames"],
                                 help="generate the lp")
        self.parser.add_argument("--dataDir", help="antares study data directory", required=True)
        self.parser.add_argument("--installDir",
                                 help="the directory where all binaries are located", required=True)
        self.parser.add_argument("--method", type=str, choices=["mpibenders", "mergeMPS", "both"],
                                 help="choose the optimization method")

        self.options_default = {
            'LOG_LEVEL': '3',
            'MAX_ITERATIONS': '-1',
            'GAP': '1e-06',
            'AGGREGATION': '0',
            'OUTPUTROOT': '.',
            'TRACE': '0',
            'DELETE_CUT': '0',
            'LOG_OUTPUT': 'COMMAND',
            'SLAVE_WEIGHT': 'CONSTANT',
            'SLAVE_WEIGHT_VALUE': '1',
            'MASTER_NAME': 'master',
            'SLAVE_NUMBER': '-1',
            'STRUCTURE_FILE': 'structure.txt',
            'INPUTROOT': '.',
            'BASIS': '1',
            'ACTIVECUTS': '0',
            'THRESHOLD_AGGREGATION': '0',
            'THRESHOLD_ITERATION': '0',
            'RAND_AGGREGATION': '0',
            'MASTER_METHOD': 'SIMPLEX',
            'CSV_NAME': 'benders_output_trace',
            'BOUND_ALPHA': '1',
            'XPRESS_TRACE': '0',
        }