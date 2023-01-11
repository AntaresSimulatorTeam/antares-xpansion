
import yaml
from antares_xpansion.xpansionConfig import ConfigParameters


class ConfigFileParser:

    def __init__(self, config_file) -> None:

        self.config_file = config_file
        self.default_install_dir = ""
        self.ANTARES_DEFAULT = "antares-solver"
        self.MERGE_MPS_DEFAULT = "merge_mps"
        self.BENDERS_MPI_DEFAULT = "benders_mpi"
        self.BENDERS_SEQUENTIAL_DEFAULT = "benders_sequential"
        self.LP_NAMER_DEFAULT = "lp_namer"
        self.STUDY_UPDATER_DEFAULT = "study_updater"
        self.ANTARES_ARCHIVE_UPDATER_DEFAULT = "antares_archive_updater"
        self.SENSITIVITY_DEFAULT = "sensitivity"
        self.AVAILABLE_SOLVERS_DEFAULT = []

    def get_config_parameters(self) -> ConfigParameters:
        with open(self.config_file) as file:
            content = yaml.full_load(file)
            if content is None:
                content = {}

            self.config = ConfigParameters(
                default_install_dir=content.get(
                    "DEFAULT_INSTALL_DIR", self.default_install_dir),
                ANTARES=content.get('ANTARES', self.ANTARES_DEFAULT),
                MERGE_MPS=content.get('MERGE_MPS', self.MERGE_MPS_DEFAULT),
                BENDERS_MPI=content.get(
                    'BENDERS_MPI', self.BENDERS_MPI_DEFAULT),
                BENDERS_SEQUENTIAL=content.get(
                    'BENDERS_SEQUENTIAL', self.BENDERS_SEQUENTIAL_DEFAULT),
                LP_NAMER=content.get('LP_NAMER', self.LP_NAMER_DEFAULT),
                STUDY_UPDATER=content.get(
                    'STUDY_UPDATER', self.STUDY_UPDATER_DEFAULT),
                ANTARES_ARCHIVE_UPDATER=content.get(
                    'ANTARES_ARCHIVE_UPDATER', self.ANTARES_ARCHIVE_UPDATER_DEFAULT),
                SENSITIVITY_EXE=content.get(
                    'SENSITIVITY', self.SENSITIVITY_DEFAULT),
                AVAILABLE_SOLVERS=content.get(
                    'AVAILABLE_SOLVER', self.AVAILABLE_SOLVERS_DEFAULT)
            )
        return self.config
