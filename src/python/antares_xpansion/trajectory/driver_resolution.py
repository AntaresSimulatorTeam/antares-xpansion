import json
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Dict

import yaml
from antares_xpansion.benders_driver import BendersDriver, SolversExe
from antares_xpansion.config_loader import XpansionSettingsReader
from antares_xpansion.logger import step_logger
from antares_xpansion.optimisation_keys import OptimisationKeys
from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.trajectory.user_input_keys import TrajectoryInputKeys as InKeys
from antares_xpansion.xpansionConfig import XpansionConfigConstants
from antares_xpansion.xpansion_study_reader import XpansionStudyReader


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
    user_input_file: Path
    # Optimization parameters (arguments)
    master_name: str
    structure_file: str
    solver: str
    problems_format: str
    cache_problems: bool
    method: str
    n_mpi: int
    oversubscribe: bool
    allow_run_as_root: bool
    master_formulation: str


class TrajectoryResolutionDriver:
    """
    A wrapper around the BendersDriver class to adapt it to the trajectory workflow
    """

    def __init__(self, data: TrajectoryResolutionData):
        self.data = data
        self.logger = step_logger(__name__, __class__.__name__)

        self.benders_driver = BendersDriver(
            SolversExe(data.benders_exe, data.frontal_exe, data.outer_loop_exe),
            data.benders_options_file.resolve(),
            data.mpi_exe,
        )

    def _read_node_to_studies(self) -> Dict[str, Path]:
        """
        Read the user input file to get the mapping of node names to study paths
        """
        node_to_studies: Dict[str, Path] = {}
        with open(self.data.user_input_file, encoding="utf-8") as file:
            user_data = yaml.full_load(file)
            studies: Dict[str, str] = user_data[InKeys.global_key()][
                InKeys.studies_key()
            ]
            for node, pathstr in studies.items():
                path = Path(pathstr)
                # Make path relative to input_root if it's not absolute
                if not path.is_absolute():
                    path = self.data.input_root / path
                node_to_studies[node] = path
        return node_to_studies

    def _clean_all_nodes_lp_directories(self):
        """
        Clean the lp directory of each node/study after resolution
        Similar to the standard memory workflow
        """
        self.logger.info("Cleaning lp directories for all trajectory nodes")
        node_to_studies = self._read_node_to_studies()
        for node, study_path in node_to_studies.items():
            # Find the output directory for this study
            # The output should be under study_path/output
            output_dir = study_path / "output"
            if output_dir.is_dir():
                # Find the last simulation output
                # Typically in the format "YYYYMMDD-HHmmSS" or similar
                # We need to find the most recent one
                output_subdirs = [d for d in output_dir.iterdir() if d.is_dir()]
                if output_subdirs:
                    # Sort by modification time, get the most recent
                    latest_output = max(output_subdirs, key=lambda d: d.stat().st_mtime)
                    self.logger.info(f"Cleaning lp directory for node '{node}' at {latest_output}")
                    # Clean the lp directory in this output
                    StudyOutputCleaner.clean_benders_step(latest_output)
                else:
                    self.logger.warning(f"No output subdirectories found for node '{node}' at {output_dir}")
            else:
                self.logger.warning(f"Output directory not found for node '{node}' at {output_dir}")

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
        options_values[OptimisationKeys.solver_name_key()] = (
            XpansionStudyReader.convert_study_solver_to_option_solver(self.data.solver)
        )
        options_values[OptimisationKeys.problems_format_key()] = (
            self.data.problems_format.upper()
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

        # Master formulation
        options_values[OptimisationKeys.master_formulation_key()] = (
            self.data.master_formulation
        )
        # Implement previously non-implemented options with safe guards
        # LAST_ITERATION_JSON_FILE: use output folder default if present
        last_iter_json = self.data.output_folder / config_defaults.LAST_ITERATION_JSON_FILE_NAME
        options_values[OptimisationKeys.last_iteration_json_file_key()] = (
            last_iter_json.resolve().__str__()
        )

        # LAST_MASTER_MPS and LAST_MASTER_BASIS: warm start files if present in input_root
        # Try solver-specific extensions for last master mps/svf
        master_last_base = f"{self.data.master_name}_last_iteration"
        candidates = [
            self.data.input_root / f"{master_last_base}.mps",
            self.data.input_root / f"{master_last_base}.svf",
            self.data.input_root / config_defaults.LAST_MASTER_MPS,
        ]
        last_master_path = next((p for p in candidates if p and p.exists()), None)
        if last_master_path is not None:
            options_values[OptimisationKeys.last_mps_master_name_key()] = (
                last_master_path.resolve().__str__()
            )

        basis_candidates = [
            self.data.input_root / f"{self.data.master_name}_last_basis.bss",
            self.data.input_root / config_defaults.LAST_MASTER_BASIS,
        ]
        last_basis_path = next((p for p in basis_candidates if p and p.exists()), None)
        if last_basis_path is not None:
            options_values[OptimisationKeys.last_master_basis_key()] = (
                last_basis_path.resolve().__str__()
            )

        # DO_OUTER_LOOP and related files
        do_outer_loop = self.data.method == "adequacy_criterion"
        options_values[OptimisationKeys.do_outer_loop_key()] = do_outer_loop
        if do_outer_loop:
            # Build paths relative to the root study directory
            root_dir = Path(root_study)
            outer_loop_file = root_dir / config_defaults.USER / config_defaults.EXPANSION / config_defaults.OUTER_LOOP_DIR / config_defaults.OUTER_LOOP_FILE
            if outer_loop_file.exists():
                options_values[OptimisationKeys.outer_loop_option_file_key()] = (
                    outer_loop_file.resolve().__str__()
                )
            area_file = root_dir / config_defaults.USER / config_defaults.EXPANSION / config_defaults.AREA_FILE
            if area_file.exists():
                options_values[OptimisationKeys.area_file_key()] = (
                    area_file.resolve().__str__()
                )

        # CACHE_PROBLEMS: use value from command line arguments
        options_values[OptimisationKeys.cache_problems_keys()] = (
            self.data.cache_problems
        )

        assert Path(os.getcwd()).resolve() == self.data.input_root.resolve()
        # Write options file for the solver
        with open(self.data.benders_options_file, "w") as options_file:
            json.dump(options_values, options_file, indent=4)

    def launch(self):
        # Run the driver from the input root
        previous_dir = os.getcwd()
        os.chdir(self.data.input_root)

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

        # Clean the lp directories of all nodes after resolution
        self._clean_all_nodes_lp_directories()

        os.chdir(previous_dir)
