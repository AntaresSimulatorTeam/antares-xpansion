"""
    parameters for an AntaresXpansion session
"""

from pathlib import Path
import argparse
import sys
import yaml

class XpansionConfig():
    """
        Class defininf the parameters for an AntaresXpansion session
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods


    def __init__(self):
    
        with open(Path.cwd() / "config.yaml") as file:
            content = yaml.full_load(file)
            # content is a Python dict
            if content is not None:
                self.ANTARES            = self._readcontent(content,'ANTARES')
                self.MERGE_MPS          = self._readcontent(content,'MERGE_MPS')
                self.BENDERS_MPI        = self._readcontent(content,'BENDERS_MPI')
                self.BENDERS_SEQUENTIAL = self._readcontent(content,'BENDERS_SEQUENTIAL')
                self.LP_NAMER           = self._readcontent(content,'LP_NAMER')
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

        self.MPI_N_PROCESSES = "4"

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

        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--step", choices=["lp", "optim", "full", "antares", "getnames"],
                                 help='step to execute ("lp", "optim", "full", "antares", "getnames")',
                                 required=True)
        self.parser.add_argument("--simulationName",
                                 help="name of the antares simulation to use. "
                                 "Must be present in the output directory")
        self.parser.add_argument("--dataDir", help="antares study data directory", required=True)
        self.parser.add_argument("--installDir",
                                 help="the directory where all binaries are located", required=True)
        self.parser.add_argument("--method", type=str,
                                 choices=["mpibenders", "mergeMPS", "both", "sequential"],
                                 help="choose the optimization method")
        self.parser.add_argument("-c",
                                 help='name of the file to use for exclusion constraints')

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

        self.settings_default = {'method' : 'benders_decomposition',
                      'uc_type' : 'expansion_fast',
                      'master' : 'integer',
                      'optimality_gap' : '0',
                      'cut_type' : 'yearly',
                      'week_selection' : 'false',
                      'max_iteration' : '+infini',
                      'relaxed_optimality_gap' : '0.01',
                      'solver' : 'Cbc',
                      'timelimit' : '+infini',
                      'additional-constraints' : ""}
    
    def _readcontent(self, content, attribute):
        if content is not None and attribute in content:
               val = content[attribute]
               return val
        else:
            raise RuntimeError(attribute + " not found. Please check file config.yaml")