from antares_xpansion.xpansionConfig import XpansionConfigConstants
from antares_xpansion.config_loader import XpansionSettingsReader
from antares_xpansion.optimisation_keys import OptimisationKeys
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.benders_driver import BendersDriver, SolversExe

from dataclasses import dataclass
from pathlib import Path
import json


@dataclass
class TrajectoryResolutionData:
    # Executables
    benders_exe: Path
    frontal_exe: Path
    outer_loop_exe: Path
    mpi_exe: Path
    # Various files and folders
    input_root: Path
    root_study: Path
    json_output_file: Path
    benders_options_file: Path
    merged_weights_file: Path
    output_folder: Path
    # Optimization parameters (arguments)
    master_name: str
    structure_file: str
    solver: str
    problems_format: str
    method: str
    n_mpi: int
    oversubscribe: bool
    allow_run_as_root: bool


class TrajectoryResolutionDriver:
    """
    A wrapper around the BendersDriver class to adapt it to the trajectory workflow
    """

    def __init__(self, data: TrajectoryResolutionData):
        self.data = data

        self.benders_driver = BendersDriver(
            SolversExe(data.benders_exe, data.frontal_exe, data.outer_loop_exe),
            data.benders_options_file.resolve(),
            data.mpi_exe,
        )

    def prepare_resolution_options_file(self):
        """
        Settings from the setting.ini file of the root study are used
        Most settings are commented out, not needed in the current simple version
        """
        root_study = self.data.root_study
        config_defaults = XpansionConfigConstants()
        config_defaults._initialize_default_values()
        root_settings_reader = XpansionSettingsReader(root_study, config_defaults)
        options_values = config_defaults.options_default

        options_values[OptimisationKeys.input_root_key()] = (
            self.data.input_root.resolve().__str__()
        )
        options_values[OptimisationKeys.outpoutroot_key()] = (
            self.data.output_folder.resolve().__str__()
        )

        # Master name and structure file
        options_values[OptimisationKeys().master_name_key()] = self.data.master_name
        options_values[OptimisationKeys().structure_file_key()] = (
            self.data.structure_file
        )
        # Weights file is mandatory
        options_values[OptimisationKeys.slave_weight_key()] = (
            self.data.merged_weights_file.resolve().__str__()
        )
        # Solver is overwritten by the global driver
        # to make sure we use same solver and problem format for all studies
        options_values[OptimisationKeys.solver_name_key()] = self.data.solver
        options_values[OptimisationKeys.problems_format_key()] = (
            self.data.problems_format
        )

        options_values[OptimisationKeys.json_file_key()] = (
            self.data.json_output_file.resolve().__str__()
        )

        # Other values are read from the root study's settings.ini
        options_values[OptimisationKeys.absolute_gap_key()] = (
            root_settings_reader.get_absolute_optimality_gap()
        )
        options_values[OptimisationKeys.relative_gap_key()] = (
            root_settings_reader.get_relative_optimality_gap()
        )
        options_values[OptimisationKeys.relaxed_gap_key()] = (
            root_settings_reader.get_relaxed_optimality_gap()
        )
        options_values[OptimisationKeys.separation_key()] = (
            root_settings_reader.get_separation()
        )
        options_values[OptimisationKeys.max_iterations_key()] = (
            root_settings_reader.get_max_iterations()
        )
        options_values[OptimisationKeys.log_level_key()] = (
            root_settings_reader.log_level()
        )
        options_values[OptimisationKeys.batch_size_key()] = (
            root_settings_reader.get_batch_size()
        )
        options_values[OptimisationKeys.time_limit_key()] = (
            root_settings_reader.timelimit()
        )
        # Irrelevant in our case, but we need to set a value.
        options_values[OptimisationKeys.slave_weight_value_key()] = 1.0

        # options_values[OptimisationKeys.last_iteration_json_file_key()] = (
        #     self.last_iteration_json_file_path()
        # )
        # options_values[OptimisationKeys.master_formulation_key()] = (
        #     root_settings_reader.get_master_formulation()
        # )
        # options_values[OptimisationKeys.last_mps_master_name_key()] = (
        #     self._config.LAST_MASTER_MPS
        # )
        # options_values[OptimisationKeys.last_master_basis_key()] = (
        #     self._config.LAST_MASTER_BASIS
        # )
        # options_values[OptimisationKeys.do_outer_loop_key()] = (
        #     self.config.method == "adequacy_criterion"
        # )
        # options_values[OptimisationKeys.outer_loop_option_file_key()] = (
        #     self.config.OUTER_LOOP_FILE
        # )
        # options_values[OptimisationKeys.area_file_key()] = self._config.AREA_FILE
        # if os.path.exists(self.outer_loop_options_path()):
        #     shutil.copy(self.outer_loop_options_path(), self._simulation_lp_path())
        # options_values[OptimisationKeys.cache_problems_keys()] = self.cache_problems()

        # Write options file for the solver
        with open(self.data.benders_options_file, "w") as options_file:
            json.dump(options_values, options_file, indent=4)

    def launch(self):
        self.prepare_resolution_options_file()
        self.benders_driver.set_custom_working_dir(self.data.input_root.resolve())
        # The cleaning step is not adapted to trajecotry mode, do not run it.
        KEEP_MPS_TRAJECTORY_MODE = True
        self.benders_driver.launch(
            self.data.output_folder,
            self.data.method,
            KEEP_MPS_TRAJECTORY_MODE,
            self.data.n_mpi,
            oversubscribe=self.data.oversubscribe,
            allow_run_as_root=self.data.allow_run_as_root,
        )
