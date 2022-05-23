"""
    parameters for an AntaresXpansion session
"""
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List

from antares_xpansion.optimisation_keys import OptimisationKeys


@dataclass
class ConfigParameters:
    default_install_dir: str
    ANTARES: str
    MERGE_MPS: str
    BENDERS_MPI: str
    BENDERS_SEQUENTIAL: str
    LP_NAMER: str
    STUDY_UPDATER: str
    SENSITIVITY_EXE: str
    AVAILABLE_SOLVERS: List[str]


@dataclass
class InputParameters:
    step: str
    simulation_name: str
    data_dir: str
    install_dir: str
    method: str
    n_mpi: int
    antares_n_cpu: int
    keep_mps: bool
    oversubscribe: bool
    allow_run_as_root: bool


class XpansionConfig:
    """
        Class defininf the parameters for an AntaresXpansion session
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods

    def __init__(self, input_parameters: InputParameters, config_parameters: ConfigParameters):

        self.input_parameters = input_parameters
        self.config_parameters = config_parameters
        self.ANTARES: str = ""
        self.MERGE_MPS: str = ""
        self.BENDERS_MPI: str = ""
        self.BENDERS_SEQUENTIAL: str = ""
        self.LP_NAMER: str = ""
        self.STUDY_UPDATER: str = ""
        self.SENSITIVITY_EXE: str = ""
        self.MPI_LAUNCHER: str = ""
        self.MPI_N: str = ""
        self.AVAILABLE_SOLVER: List[str]

        self._get_config_values()

        self._get_parameters_from_arguments()

        self._initialize_default_values()

    def _get_parameters_from_arguments(self):
        self.step = self.input_parameters.step
        self.simulation_name = self.input_parameters.simulation_name
        self.data_dir = str(Path(self.input_parameters.data_dir).absolute())
        self.install_dir = self._get_install_dir(
            self.input_parameters.install_dir)
        self.method = self.input_parameters.method
        self.n_mpi = self.input_parameters.n_mpi
        self.antares_n_cpu = self.input_parameters.antares_n_cpu
        self.keep_mps = self.input_parameters.keep_mps
        self.oversubscribe = self.input_parameters.oversubscribe
        self.allow_run_as_root = self.input_parameters.allow_run_as_root

    def _get_install_dir(self, install_dir):
        if install_dir is None:
            return self._initialize_install_dir_with_default_value()
        else:
            return self._initialize_install_dir_with_absolute_path(install_dir)

    @staticmethod
    def _initialize_install_dir_with_absolute_path(install_dir):
        if not Path.is_absolute(Path(install_dir)):
            return os.path.join(Path.cwd(), Path(install_dir))
        else:
            return install_dir

    def _initialize_install_dir_with_default_value(self):

        if getattr(sys, 'frozen', False):
            install_dir_inside_package = Path(
                os.path.abspath(__file__)).parent.parent / "bin"
            install_dir_next_to_package = Path(sys.executable).parent / "bin"
            if Path.is_dir(install_dir_inside_package):
                return install_dir_inside_package
            else:
                return install_dir_next_to_package
        elif __file__:
            return self.default_install_dir

    def _initialize_default_values(self):
        self._set_constants()
        self._set_default_options()
        self._set_default_settings()

    def _set_constants(self):
        # TODO move self.SETTINGS, self.GENERAL_DATA_INI, self.OUTPUT into antares driver
        self.SETTINGS = 'settings'
        self.GENERAL_DATA_INI = 'generaldata.ini'
        self.OUTPUT = 'output'

        self.USER = 'user'
        self.EXPANSION = 'expansion'
        self.SENSITIVITY_DIR = 'sensitivity'
        self.CAPADIR = 'capa'
        self.NB_YEARS = 'nbyears'
        self.SETTINGS_INI = 'settings.ini'
        self.CANDIDATES_INI = 'candidates.ini'
        self.UC_TYPE = 'uc_type'
        self.EXPANSION_ACCURATE = 'expansion_accurate'
        self.EXPANSION_FAST = 'expansion_fast'
        self.OPTIONS_JSON = 'options.json'
        self.LAUNCHER_OPTIONS_JSON = 'launcher_options.json'
        self.JSON_NAME = "out.json"
        self.LAST_ITERATION_JSON_FILE_NAME = "last_iteration.json"
        self.JSON_SENSITIVITY_IN = "sensitivity_in.json"
        self.JSON_SENSITIVITY_OUT = "sensitivity_out.json"
        self.SENSITIVITY_LOG_FILE = "sensitivity_log.txt"
        self.LAST_MASTER_MPS = "master_last_iteration"
        self.LAST_MASTER_BASIS = "master_last_basis.bss"

    def _set_default_settings(self):
        self.settings_default = {
            "method": "benders_decomposition",
            "uc_type": "expansion_fast",
            "master": "integer",
            "optimality_gap": "1",
            "relative_gap": "1e-12",
            "cut_type": "yearly",
            "week_selection": "false",
            "max_iteration": "+infini",
            "relaxed_optimality_gap": "1e-4",
            "initial_master_relaxation": "true",
            "solver": "Cbc",
            "timelimit": "+infini",
            "additional-constraints": "",
            "yearly-weights": "",
            "log_level": "0",
        }

    def _set_default_options(self):
        self.options_default = {
            OptimisationKeys.max_iterations_key(): self.max_iterations_default_value(),
            OptimisationKeys.absolute_gap_key(): self.absolute_gap_default_value(),
            OptimisationKeys.relative_gap_key(): self.relative_gap_default_value(),
            OptimisationKeys.relaxed_gap_key(): self.relaxed_gap_default_value(),
            OptimisationKeys.initial_master_relaxation_key(): self.initial_master_relaxation_default_value(),
            OptimisationKeys.aggregation_key(): self.aggregation_default_value(),
            OptimisationKeys.outpoutroot_key(): self.outpoutroot_default_value(),
            OptimisationKeys.trace_key(): self.trace_default_value(),
            OptimisationKeys.slave_weight_key(): self.slave_weight_default_value(),
            OptimisationKeys.slave_weight_value_key(): self.slave_weight_value_default_value(),
            OptimisationKeys.master_name_key(): self.master_name_default_value(),
            OptimisationKeys.structure_file_key(): self.structure_file_default_value(),
            OptimisationKeys.input_root_key(): self.input_root_default_value(),
            OptimisationKeys.csv_name_key(): self.csv_name_default_value(),
            OptimisationKeys.bound_alpha_key(): self.bound_alpha_default_value(),
        }

    def bound_alpha_default_value(self):
        return True

    def csv_name_default_value(self):
        return "benders_output_trace"

    def input_root_default_value(self):
        return "."

    def structure_file_default_value(self):
        return "structure.txt"

    def master_name_default_value(self):
        return "master"

    def slave_weight_value_default_value(self):
        return "1"

    def slave_weight_default_value(self):
        return "CONSTANT"

    def trace_default_value(self):
        return True

    def outpoutroot_default_value(self):
        return "."

    def aggregation_default_value(self):
        return False

    def relative_gap_default_value(self):
        return "1e-12"

    def absolute_gap_default_value(self):
        return "1"
    
    def relaxed_gap_default_value(self):
        return "1e-4"

    def max_iterations_default_value(self):
        return "-1"
    
    def initial_master_relaxation_default_value(self):
        return False

    def _get_config_values(self):

        self.default_install_dir = self.config_parameters.default_install_dir
        self.ANTARES = self.config_parameters.ANTARES
        self.MERGE_MPS = self.config_parameters.MERGE_MPS
        self.BENDERS_MPI = self.config_parameters.BENDERS_MPI
        self.BENDERS_SEQUENTIAL = self.config_parameters.BENDERS_SEQUENTIAL
        self.LP_NAMER = self.config_parameters.LP_NAMER
        self.STUDY_UPDATER = self.config_parameters.STUDY_UPDATER
        self.SENSITIVITY_EXE = self.config_parameters.SENSITIVITY_EXE
        self.AVAILABLE_SOLVER = self.config_parameters.AVAILABLE_SOLVERS
