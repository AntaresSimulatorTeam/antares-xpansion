"""
    parameters for an AntaresXpansion session
"""

from pathlib import Path
import sys
import yaml


class XpansionConfig():
    """
        Class defininf the parameters for an AntaresXpansion session
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods

    def __init__(self, configfile: Path):

        with open(configfile) as file:
            content = yaml.full_load(file)
            if content is not None:
                self.ANTARES = content.get('ANTARES', "antares-solver")
                self.MERGE_MPS = content.get('MERGE_MPS', "merge_mps")
                self.BENDERS_MPI = content.get('BENDERS_MPI', "bender_mpi")
                self.BENDERS_SEQUENTIAL = content.get('BENDERS_SEQUENTIAL', "benders_sequential")
                self.LP_NAMER = content.get('LP_NAMER', "lp_namer")
            else:
                raise RuntimeError("Please check file config.yaml, content is empty")

        if sys.platform.startswith("win32"):
            self.MPI_LAUNCHER = "mpiexec"
            self.MPI_N = "-n"
        elif sys.platform.startswith("linux"):
            self.MPI_LAUNCHER = "mpirun"
            self.MPI_N = "-np"
        else:
            print("WARN: No mpi launcher was defined!")

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
        self.MPS_TXT = "mps.txt"

        self.options_default = {
            'LOG_LEVEL': '3',
            'MAX_ITERATIONS': '-1',
            'GAP': '1e-06',
            'AGGREGATION': '0',
            'OUTPUTROOT': '.',
            'TRACE': '1',
            'DELETE_CUT': '0',
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
            'CSV_NAME': 'benders_output_trace',
            'BOUND_ALPHA': '1',
        }

        self.settings_default = {'method': 'benders_decomposition',
                                 'uc_type': 'expansion_fast',
                                 'master': 'integer',
                                 'optimality_gap': '0',
                                 'cut_type': 'yearly',
                                 'week_selection': 'false',
                                 'max_iteration': '+infini',
                                 'relaxed_optimality_gap': '0.01',
                                 'solver': 'Cbc',
                                 'timelimit': '+infini',
                                 'additional-constraints': ""}
