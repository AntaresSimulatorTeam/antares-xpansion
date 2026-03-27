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
        self.PRESOLVE_DEFAULT = "presolve"
        self.STUDY_UPDATER_DEFAULT = "study_updater"
        self.FULL_RUN_DEFAULT = "full_run"
        self.OUTER_LOOP_DEFAULT = "outer_loop"
        self.ANTARES_ARCHIVE_UPDATER_DEFAULT = "antares_archive_updater"
        self.ANTARES_PROBLEM_GENERATOR_DEFAULT = "antares-problem-generator"
        self.ANTARES_MODELER_DEFAULT = "antares-modeler"
        self.SENSITIVITY_DEFAULT = "sensitivity"
        self.MPIEXEC_DEFAULT = "mpiexec"
        self.AVAILABLE_SOLVERS_DEFAULT = []
        # Trajectory investment C++ executables
        self.MULTIPLE_PROBLEM_GEN_DEFAULT = "multiple_problem_generation"
        self.MERGE_MASTER_MPS_DEFAULT = "merge_master_mps"
        self.MERGE_WEIGHTS_TRAJECTORY_DEFAULT = "merge_weights_trajectory"

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
                PRESOLVE=content.get("PRESOLVE", self.PRESOLVE_DEFAULT),
                STUDY_UPDATER=content.get("STUDY_UPDATER", self.STUDY_UPDATER_DEFAULT),
                FULL_RUN=content.get("FULL_RUN", self.FULL_RUN_DEFAULT),
                OUTER_LOOP=content.get("OUTER_LOOP", self.FULL_RUN_DEFAULT),
                ANTARES_ARCHIVE_UPDATER=content.get(
                    "ANTARES_ARCHIVE_UPDATER", self.ANTARES_ARCHIVE_UPDATER_DEFAULT
                ),
                ANTARES_PROBLEM_GENERATOR=content.get(
                    "ANTARES_PROBLEM_GENERATOR", self.ANTARES_PROBLEM_GENERATOR_DEFAULT
                ),
                ANTARES_MODELER=content.get(
                    "ANTARES_MODELER", self.ANTARES_MODELER_DEFAULT
                ),
                SENSITIVITY_EXE=content.get("SENSITIVITY", self.SENSITIVITY_DEFAULT),
                MPIEXEC=content.get("mpiexec", self.MPIEXEC_DEFAULT),
                AVAILABLE_SOLVERS=content.get(
                    "AVAILABLE_SOLVER", self.AVAILABLE_SOLVERS_DEFAULT
                ),
                MULTIPLE_PROBLEM_GEN=content.get(
                    "MULTIPLE_PROBLEM_GENERATION", self.MULTIPLE_PROBLEM_GEN_DEFAULT
                ),
                MERGE_MASTER_MPS=content.get(
                    "MERGE_MASTER_MPS", self.MERGE_MASTER_MPS_DEFAULT
                ),
                MERGE_WEIGHTS_TRAJECTORY=content.get(
                    "MERGE_WEIGHTS_TRAJECTORY", self.MERGE_WEIGHTS_TRAJECTORY_DEFAULT
                ),
            )
        return self.config
