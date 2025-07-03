from dataclasses import dataclass
from pathlib import Path

from antares_xpansion.xpansionConfig import ConfigParameters


@dataclass
class TrajectoryInputParameters:
    step: str
    input_root: Path
    input_file: Path
    memory: bool
    # Relevant for resolution only
    method: str
    n_mpi: int
    oversubscribe: bool
    allow_run_as_root: bool


class TrajectoryConfigDefaults:
    def __init__(self):
        self._set_constants()

    def _set_constants(self):
        self.INTERMEDIARY_FOLDER = "intermediary_files"
        self.OUTPUT_FOLDER = "output"
        # Multiple problem generation input files
        self.MPG_INPUT_FILE = "mpg_input.txt"
        self.MPG_WEIGHTS_FILE = "mpg_weights.txt"
        self.MPG_CONSTRAINTS_FILE = "mpg_additional_constraints.txt"
        # Intermediary files
        self.MASTER_MERGER_INFO_FILE = "master_merger_info.json"
        self.NODAL_LP_INFO_FILE = "nodal_lp_info.json"
        self.MERGE_MASTER_OPTIONS_FILE = "options_merge_master.json"
        # Merge master output file
        self.MERGED_MASTER = "merged_master"
        self.MERGED_STRUCTURE = "merged_structure.txt"
        # Merge weights output file
        self.MERGED_WEIGHTS = "merged_weights.txt"


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

    def get_executable_path(self, exe_name: str):
        assert hasattr(self, "default_install_dir")
        return Path(self.default_install_dir) / exe_name

    def _get_input_parameters(self):
        self.step = self.input_parameters.step
        self.input_root = self.input_parameters.input_root
        self.input_file = self.input_parameters.input_file
        self.memory = self.input_parameters.memory
        # Resolution args
        self.method = self.input_parameters.method
        self.n_mpi = self.input_parameters.n_mpi
        self.oversubsribe = self.input_parameters.oversubscribe
        self.allow_run_as_root = self.input_parameters.allow_run_as_root

    def _get_installation_parameters(self):
        # Single xpansion components, perhaps all are not necesary in the trajectory execution.
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
