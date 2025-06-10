from dataclasses import dataclass
from pathlib import Path

from antares_xpansion.xpansionConfig import ConfigParameters


@dataclass
class TrajectoryInputParameters:
    step: str
    input_root: Path
    input_file: Path
    memory: bool


class TrajectoryConfigDefaults:
    def __init__(self):
        self._set_constants()

    def _set_constants(self):
        self.INTERMEDIARY_FOLDER = "intermediary_files"
        # Multiple problem generation input files
        self.MPG_INPUT_FILE = "mpg_input.txt"
        self.MPG_WEIGHTS_FILE = "mpg_weights.txt"
        self.MPG_CONSTRAINTS_FILE = "mpg_additional_constraints.txt"
        # Intermediary files
        self.MASTER_MERGER_INFO_FILE = "master_merger_info.json"
        self.NODAL_LP_INFO_FILE = "nodal_lp_info.json"
        # Merge master output file
        self.MERGED_MASTER = "merged_master"
        self.MERGED_STRUCTURE = "merged_structure.txt"


class TrajectoryConfig(TrajectoryConfigDefaults):
    def __init__(
        self,
        input_parameters: TrajectoryInputParameters,
        install_parameters: ConfigParameters,
    ):
        super().__init__()
        # Args inputted by the user
        self.input_parameters = input_parameters
        # Installation configuration
        self.config_parameters = install_parameters

        self._get_input_parameters()
        self._get_installation_parameters()

    def _get_input_parameters(self):
        self.step = self.input_parameters.step
        self.input_root = self.input_parameters.input_root
        self.input_file = self.input_parameters.input_file
        self.memory = self.input_parameters.memory

    def _get_installation_parameters(self):
        self.default_install_dir = self.config_parameters.default_install_dir
        self.ANTARES = self.config_parameters.ANTARES
        self.MERGE_MPS = self.config_parameters.MERGE_MPS
        self.BENDERS = self.config_parameters.BENDERS
        self.LP_NAMER = self.config_parameters.LP_NAMER
        self.STUDY_UPDATER = self.config_parameters.STUDY_UPDATER
        self.FULL_RUN = self.config_parameters.FULL_RUN
        self.OUTER_LOOP = self.config_parameters.OUTER_LOOP
        self.ANTARES_ARCHIVE_UPDATER = self.config_parameters.ANTARES_ARCHIVE_UPDATER
        self.SENSITIVITY_EXE = self.config_parameters.SENSITIVITY_EXE
        self.MPIEXEC = self.config_parameters.MPIEXEC
        self.AVAILABLE_SOLVER = self.config_parameters.AVAILABLE_SOLVERS
        # Trajectory executables
        self.MULTIPLE_PROBLEM_GEN = self.config_parameters.MULTIPLE_PROBLEM_GEN
        self.MERGE_MASTER_MPS = self.config_parameters.MERGE_MASTER_MPS
        self.MERGE_WEIGHTS = self.config_parameters.MERGE_WEIGHTS_TRAJECTORY
