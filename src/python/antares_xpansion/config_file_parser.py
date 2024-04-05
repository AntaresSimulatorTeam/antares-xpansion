import yaml
from antares_xpansion.xpansionConfig import ConfigParameters


class ConfigFileParser:

    def __init__(self, config_file) -> None:

        self.config_file = config_file
        self.default_install_dir = ""
        self.ANTARES_DEFAULT = "antares-solver"
        self.MERGE_MPS_DEFAULT = "merge_mps"
        self.BENDERS_DEFAULT = "benders"
        self.LP_NAMER_DEFAULT = "lp_namer"
        self.STUDY_UPDATER_DEFAULT = "study_updater"
        self.FULL_RUN_DEFAULT = "full_run"
        self.ANTARES_ARCHIVE_UPDATER_DEFAULT = "antares_archive_updater"
        self.SENSITIVITY_DEFAULT = "sensitivity"
        self.MPIEXEC = "mpiexec"
        self.AVAILABLE_SOLVERS_DEFAULT = []

    def get_config_parameters(self) -> ConfigParameters:
        with open(self.config_file) as file:
            content = yaml.full_load(file)
            if content is None:
                content = {}

            self.config = ConfigParameters(
                default_install_dir=content.get(
                    "DEFAULT_INSTALL_DIR", self.default_install_dir
                ),
                ANTARES=content.get("ANTARES", self.ANTARES_DEFAULT),
                MERGE_MPS=content.get("MERGE_MPS", self.MERGE_MPS_DEFAULT),
                BENDERS=content.get("BENDERS", self.BENDERS_DEFAULT),
                LP_NAMER=content.get("LP_NAMER", self.LP_NAMER_DEFAULT),
                STUDY_UPDATER=content.get("STUDY_UPDATER", self.STUDY_UPDATER_DEFAULT),
                FULL_RUN=content.get("FULL_RUN", self.FULL_RUN_DEFAULT),
                ANTARES_ARCHIVE_UPDATER=content.get(
                    "ANTARES_ARCHIVE_UPDATER", self.ANTARES_ARCHIVE_UPDATER_DEFAULT
                ),
                SENSITIVITY_EXE=content.get("SENSITIVITY", self.SENSITIVITY_DEFAULT),
                MPIEXEC=content.get("mpiexec", self.MPIEXEC_DEFAULT),
                AVAILABLE_SOLVERS=content.get(
                    "AVAILABLE_SOLVER", self.AVAILABLE_SOLVERS_DEFAULT
                ),
            )
        return self.config
