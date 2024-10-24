"""
    Class to work on config
"""

import json
import os
import re
import shutil
import sys
import zipfile
from pathlib import Path

from antares_xpansion.chronicles_checker import ChronicleChecker
from antares_xpansion.general_data_reader import GeneralDataIniReader
from antares_xpansion.input_checker import check_candidates_file, check_options
from antares_xpansion.launcher_options_default_value import LauncherOptionsDefaultValues
from antares_xpansion.launcher_options_keys import LauncherOptionsKeys
from antares_xpansion.logger import step_logger
from antares_xpansion.optimisation_keys import OptimisationKeys
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.xpansion_study_reader import XpansionStudyReader


class NTCColumnConstraintError(Exception):
    pass


def read_antares_version(study_path):
    matcher = re.compile(r"(version = )(\d+)")
    with open(Path(study_path) / "study.antares") as antares_file:
        for line in antares_file.readlines():
            result = matcher.match(line.strip())
            if result:
                return int(result.group(2))
    return 720


class ConfigLoader:
    """
    Class to control the execution of the optimization session
    """

    def __init__(self, config: XpansionConfig):
        """
        Initialise driver with a given antaresXpansion configuration,
        the system platform and parses the arguments
        :param config: configuration to use for the optimization
        :type config: XpansionConfig object
        """
        self.platform = sys.platform
        self.logger = step_logger(__name__, __class__.__name__)

        self._config = config
        self._last_study = None
        if self._config.step == "resume":
            self._config.simulation_name = (
                LauncherOptionsDefaultValues.DEFAULT_SIMULATION_NAME()
            )
            self._xpansion_simulation_name = self._config.simulation_name
            self._restore_launcher_options()
        else:
            self._set_simulation_name()

        self.candidates_list = []
        self._verify_settings_ini_file_exists()

        self.active_years = GeneralDataIniReader(
            Path(self.general_data())
        ).get_active_years()

        self.options = self._get_options_from_settings_inifile()

        self.check_candidates_file_format()
        self.check_settings_file_format()
        antares_version = read_antares_version(self._config.data_dir)
        self.check_NTC_column_constraints(antares_version)

    def _set_simulation_name(self):
        if not self._config.simulation_name:
            raise ConfigLoader.MissingSimulationName(
                "Missing argument simulationName")
        elif self._config.simulation_name == "last":
            self._xpansion_simulation_name = self._config.simulation_name

        else:
            tmp_path = Path(self.antares_output()) / \
                self._config.simulation_name
            if self.is_antares_study_output(tmp_path):
                self._last_study = tmp_path
                self._set_xpansion_simulation_name()
            else:
                raise ConfigLoader.InvalidSimulationName(
                    f"{tmp_path} is not a valid Antares output")

    def _restore_launcher_options(self):
        with open(self.launcher_options_file_path(), "r") as launcher_options:
            options = json.load(launcher_options)

        self._config.method = options[LauncherOptionsKeys.method_key()]
        self._config.n_mpi = options[LauncherOptionsKeys.n_mpi_key()]
        self._config.antares_n_cpu = options[LauncherOptionsKeys.antares_n_cpu_key(
        )]
        self._config.keep_mps = options[LauncherOptionsKeys.keep_mps_key()]
        self._config.oversubscribe = options[LauncherOptionsKeys.oversubscribe_key(
        )]
        self._config.allow_run_as_root = options[
            LauncherOptionsKeys.allow_run_as_root_key()
        ]
        self._config.memory = options[
            LauncherOptionsKeys.memory_key()
        ]

    def _verify_settings_ini_file_exists(self):
        if not os.path.isfile(self._get_settings_ini_filepath()):
            raise ConfigLoader.MissingFile(
                " %s was not retrieved." % self._get_settings_ini_filepath()
            )

    def _get_options_from_settings_inifile(self):
        with open(self._get_settings_ini_filepath(), "r") as file_l:
            options = dict(
                {
                    line.strip()
                    .split("=")[0]
                    .strip(): line.strip()
                    .split("=")[1]
                    .strip()
                    for line in file_l.readlines()
                    if line.strip()
                }
            )
        return options

    def check_candidates_file_format(self):
        if not os.path.isfile(self.candidates_ini_filepath()):
            raise ConfigLoader.MissingFile(
                " %s was not retrieved." % self.candidates_ini_filepath()
            )

        check_candidates_file(
            Path(self.candidates_ini_filepath()), Path(self.capacity_file(""))
        )

    def check_settings_file_format(self):
        check_options(self.options)
        self._verify_solver()
        self._verify_additional_constraints_file()

    def antares_output(self):
        """
        returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir(), self._config.OUTPUT))

    def exe_path(self, exe):
        """
        prefixes the input exe with the install directory containing the binaries

        :param exe: executable name

        :return: path to specified executable
        """
        return os.path.normpath(os.path.join(self._config.install_dir, exe))

    def data_dir(self):
        """
        returns path to the data directory
        """
        return self._config.data_dir

    def weight_file_name(self):
        return self.options.get(
            "yearly-weights", self._config.settings_default["yearly-weights"]
        )

    def general_data(self):
        """
        returns path to general data ini file
        """
        return os.path.normpath(
            os.path.join(
                self.data_dir(), self._config.SETTINGS, self._config.GENERAL_DATA_INI
            )
        )

    def _get_settings_ini_filepath(self):
        """
        returns path to setting ini file
        """
        return self._get_path_from_file_in_xpansion_dir(self._config.SETTINGS_INI)

    def candidates_ini_filepath(self):
        """
        returns path to candidates ini file
        """
        return self._get_path_from_file_in_xpansion_dir(self._config.CANDIDATES_INI)

    def _get_path_from_file_in_xpansion_dir(self, filename):
        return os.path.normpath(
            os.path.join(
                self.data_dir(), self._config.USER, self._config.EXPANSION, filename
            )
        )

    def _get_weight_file_path_in_weights_dir(self, filename):
        return self._get_path_from_file_in_xpansion_dir(os.path.normpath(os.path.join(self._config.WEIGHTS, filename)))

    def _get_constraints_file_path_in_constraints_dir(self, filename):
        return self._get_path_from_file_in_xpansion_dir(os.path.normpath(os.path.join(self._config.CONSTRAINTS, filename)))

    def capacity_file(self, filename):
        """
        returns path to input capacity file
        """
        return os.path.normpath(
            os.path.join(
                self.data_dir(),
                self._config.USER,
                self._config.EXPANSION,
                self._config.CAPADIR,
                filename,
            )
        )

    def json_sensitivity_in_path(self):
        """
        returns path to sensitivity input file
        """
        return os.path.normpath(
            os.path.join(
                self.data_dir(),
                self._config.USER,
                self._config.EXPANSION,
                self._config.SENSITIVITY_DIR,
                self._config.JSON_SENSITIVITY_IN,
            )
        )

    def weights_file_path(self):
        """
        returns the path to a yearly-weights file

        :return: path to input yearly-weights file
        """

        yearly_weights_filename = self.weight_file_name()
        if yearly_weights_filename:
            return self._get_weight_file_path_in_weights_dir(yearly_weights_filename)
        else:
            return ""

    def get_absolute_optimality_gap(self):
        """
        returns the absolute optimality gap read from the settings file
        :return: gap value or 0 if the gap is negative
        """
        if "optimality_gap" not in self.options:
            self.logger.info(
                f" optimality_gap not defined, default value = {self._config.settings_default['optimality_gap']} used"
            )
        abs_optimality_gap_str = self.options.get(
            "optimality_gap", self._config.settings_default["optimality_gap"]
        )

        return float(abs_optimality_gap_str) if float(abs_optimality_gap_str) > 0 else 0

    def get_relative_optimality_gap(self):
        """
        returns the relative optimality gap read from the settings file
        :return: gap value or 1e-12 if the value is set to a lower value than 1e-12
        """
        rel_optimality_gap_str = self.options.get(
            "relative_gap", self._config.settings_default["relative_gap"]
        )

        return (
            float(rel_optimality_gap_str)
            if float(rel_optimality_gap_str) > 1e-12
            else 1e-12
        )

    def get_relaxed_optimality_gap(self):
        """
        returns the relaxed optimality gap read from the settings file
        :return: gap value or 1e-12 if the value is set to a lower value than 1e-12
        """
        relaxed_gap_str = self.options.get(
            "relaxed_optimality_gap",
            self._config.settings_default["relaxed_optimality_gap"],
        )

        return float(relaxed_gap_str) if float(relaxed_gap_str) > 1e-12 else 1e-12

    def get_max_iterations(self):
        """
        prints and returns the maximum iterations read from the settings file

        :return: max iterations value or -1 if the parameter is set to +Inf or +infini
        """
        max_iterations_str = self.options.get(
            "max_iteration", self._config.settings_default["max_iteration"]
        )

        return (
            float(max_iterations_str)
            if ((max_iterations_str != "+Inf") and (max_iterations_str != "+infini"))
            else -1
        )

    def get_master_formulation(self):
        """
        return master formulation read from the settings file
        """
        return self.options.get("master", self._config.settings_default["master"])

    def get_separation(self):
        """
        return the separation parameter read from the settings file
        """
        separation_parameter_str = self.options.get(
            "separation_parameter",
            self._config.settings_default["separation_parameter"],
        )

        return float(separation_parameter_str)

    def get_batch_size(self):
        """
        return the batch_size read from the settings file
        """
        batch_size_str = self.options.get(
            "batch_size",
            self._config.settings_default["batch_size"],
        )

        return int(batch_size_str)

    def additional_constraints(self):
        """
        returns path to additional constraints file
        """
        additional_constraints_filename = self.options.get(
            "additional-constraints",
            self._config.settings_default["additional-constraints"],
        )

        if additional_constraints_filename == "":
            return ""
        return self._get_constraints_file_path_in_constraints_dir(additional_constraints_filename)

    def memory(self):
        return self._config.memory

    def simulation_lp_path(self):
        return self._simulation_lp_path()

    def _simulation_lp_path(self):
        return Path(self.xpansion_simulation_output()) / "lp"

    def xpansion_simulation_output(self) -> Path:
        if self._xpansion_simulation_name == "last":
            self._set_last_simulation_name()
        return self._xpansion_simulation_name

    def _verify_additional_constraints_file(self):
        if self.options.get("additional-constraints", "") != "":
            additional_constraints_path = self.additional_constraints()
            if not os.path.isfile(additional_constraints_path):
                self.logger.error(
                    "Illegal value: %s is not an existent additional-constraints file"
                    % additional_constraints_path
                )
                sys.exit(1)

    def _verify_solver(self):

        if "solver" not in self.options:
            default_solver = self._config.settings_default["solver"]
            self.logger.info(
                f"No solver defined in user/expansion/settings.ini. {default_solver} used"
            )
            self.options["solver"] = default_solver
        else:
            try:
                XpansionStudyReader.check_solver(
                    self.options["solver"], self._config.AVAILABLE_SOLVER
                )
            except XpansionStudyReader.BaseException as e:
                self.logger.error(e)
                sys.exit(1)

    def simulation_output_path(self) -> Path:
        if self._xpansion_simulation_name == "last":
            self._set_last_simulation_name()

        return self._last_study

    def benders_pre_actions(self):
        self.copy_area_file_to_lpdir()
        self.save_launcher_options()
        if self._config.step != "resume":  # expansion dir alaready in resume mode
            self.create_expansion_dir()
        self._set_options_for_benders_solver()

    def copy_area_file_to_lpdir(self):
        area_file = self.xpansion_simulation_output() / "area.txt"
        if area_file.exists():
            shutil.copy(area_file, self._simulation_lp_path())

    def save_launcher_options(self):
        options = {}
        options[LauncherOptionsKeys.method_key()] = self.method()
        options[LauncherOptionsKeys.n_mpi_key()] = self.n_mpi()
        options[LauncherOptionsKeys.antares_n_cpu_key()] = self.antares_n_cpu()
        options[LauncherOptionsKeys.keep_mps_key()] = self.keep_mps()
        options[LauncherOptionsKeys.oversubscribe_key()] = self.oversubscribe()
        options[LauncherOptionsKeys.allow_run_as_root_key()
                ] = self.allow_run_as_root()

        with open(self.launcher_options_file_path(), "w") as launcher_options:
            json.dump(options, launcher_options, indent=4)

    def launcher_options_file_path(self):
        return os.path.normpath(
            os.path.join(self._simulation_lp_path(),
                         self._config.LAUNCHER_OPTIONS_JSON)
        )

    def create_expansion_dir(self):
        expansion_dir = self.expansion_dir()
        if os.path.isdir(expansion_dir):
            shutil.rmtree(expansion_dir)
        os.makedirs(expansion_dir)

    def _create_sensitivity_dir(self):
        sensitivity_dir = self._sensitivity_dir()
        if os.path.isdir(sensitivity_dir):
            shutil.rmtree(sensitivity_dir)
        os.makedirs(sensitivity_dir)

    def expansion_dir(self):
        return os.path.normpath(
            os.path.join(self.xpansion_simulation_output(), "expansion")
        )

    def _sensitivity_dir(self):
        return self.xpansion_simulation_output() / "sensitivity"

    def _set_options_for_benders_solver(self):
        """
        generates a default option file for the solver
        """
        # computing the weight of slaves
        options_values = self._config.options_default

        options_values[OptimisationKeys.slave_weight_value_key()] = len(
            self.active_years
        )
        options_values[OptimisationKeys.json_file_key()
                       ] = self.json_file_path()
        options_values[
            OptimisationKeys.last_iteration_json_file_key()
        ] = self.last_iteration_json_file_path()
        options_values[
            OptimisationKeys.absolute_gap_key()
        ] = self.get_absolute_optimality_gap()
        options_values[
            OptimisationKeys.relative_gap_key()
        ] = self.get_relative_optimality_gap()
        options_values[
            OptimisationKeys.relaxed_gap_key()
        ] = self.get_relaxed_optimality_gap()
        options_values[
            OptimisationKeys.master_formulation_key()
        ] = self.get_master_formulation()
        options_values[OptimisationKeys.separation_key()
                       ] = self.get_separation()
        options_values[
            OptimisationKeys.max_iterations_key()
        ] = self.get_max_iterations()
        options_values[
            OptimisationKeys.solver_name_key()
        ] = XpansionStudyReader.convert_study_solver_to_option_solver(
            self.options.get("solver", "Cbc")
        )

        if self.weight_file_name():
            options_values[
                OptimisationKeys.slave_weight_key()
            ] = self.weight_file_name()
        options_values[OptimisationKeys.time_limit_key()] = self.timelimit()
        options_values[OptimisationKeys.log_level_key()] = self.log_level()
        options_values[
            OptimisationKeys.last_mps_master_name_key()
        ] = self._config.LAST_MASTER_MPS
        options_values[OptimisationKeys.last_master_basis_key()] = self._config.LAST_MASTER_BASIS
        options_values[OptimisationKeys.batch_size_key()] = self.get_batch_size()
        options_values[OptimisationKeys.do_outer_loop_key()] = (
            self._config.method == "adequacy_criterion"
        )
        options_values[OptimisationKeys.outer_loop_option_file_key()] = (
            self._config.OUTER_LOOP_FILE
        )
        options_values[OptimisationKeys.area_file_key()] = (
            self._config.AREA_FILE
        )
        if os.path.exists(self.outer_loop_options_path()):
            shutil.copy(self.outer_loop_options_path(), self._simulation_lp_path())

        # generate options file for the solver
        with open(self.options_file_path(), "w") as options_file:
            json.dump(options_values, options_file, indent=4)

    def options_file_path(self):
        return os.path.normpath(
            os.path.join(self._simulation_lp_path(), self._config.OPTIONS_JSON)
        )

    def options_file_name(self):
        return self._config.OPTIONS_JSON

    def _set_last_simulation_name(self):
        """
        return last simulation name
        """
        self._last_study = self.last_modified_study(Path(self.antares_output()))

        self._set_xpansion_simulation_name()
    class NotAnXpansionOutputDir(Exception):
        pass

    def _set_xpansion_simulation_name(self):
        xpansion_dir_suffix ="-Xpansion"
        self._xpansion_simulation_name = self._last_study

        if self.step() in ["resume", "sensitivity"] :
            self._xpansion_simulation_name = self._last_study
            if self.is_zip(self._last_study):
                self._xpansion_simulation_name = self._last_study.parent / self._last_study.stem
                with zipfile.ZipFile(self._last_study, 'r') as output_zip:
                    output_zip.extractall(self._xpansion_simulation_name)
        elif self.step() == "benders":
            if self.is_zip(self._last_study):
                raise ConfigLoader.NotAnXpansionOutputDir(
                    f"Error! {self._last_study} is not an Xpansion output directory"
                )

        elif self.step() == "problem_generation":
            if not self.is_zip(self._last_study):
                if(not self._last_study.name.endswith(xpansion_dir_suffix)):
                    raise ConfigLoader.NotAnXpansionOutputDir(f"Error! {self._last_study} is not an Xpansion output directory")
                else:
                    self._xpansion_simulation_name = self._last_study
                    self._last_study = self._last_study.parent / (
                            self._last_study.stem[: -len(xpansion_dir_suffix)] + ".zip"
                    )
        elif self.step() == "full" and self.memory():
            self._xpansion_simulation_name = self._last_study
        else:
            self._xpansion_simulation_name = self._last_study.parent / \
                (self._last_study.stem+"-Xpansion")

    def is_zip(self, study):
        _, ext = os.path.splitext(study)
        return ext == ".zip" 

    def update_last_study_with_sensitivity_results(self):
        if self.is_zip(self._last_study):
            os.remove(self._last_study)
            shutil.make_archive(self._last_study.parent / self._last_study.stem, 'zip', self._xpansion_simulation_name)  
            if(os.path.exists(self._xpansion_simulation_name)):
                shutil.rmtree(self._xpansion_simulation_name)

    def is_antares_study_output(self, study: Path):
        _, ext = os.path.splitext(study)
        if self.memory() and '-Xpansion' not in study.name:  # memory mode we work with files essentially
            return os.path.isdir(study)
        else:
            return ext == ".zip" or os.path.isdir(study)

    def last_modified_study(self, root_dir:Path)-> Path: 
        list_dir = os.listdir(root_dir)
        list_of_studies = filter(
            lambda x: self.is_antares_study_output(root_dir / x), list_dir
        )
        # Sort list of files based on last modification time in ascending order
        sort_studies = sorted(
            list_of_studies,
            key=lambda x: os.path.getmtime(
                os.path.join(root_dir, x)),
        )
        if len(sort_studies) == 0:
            raise ConfigLoader.MissingAntaresOutput("No Antares output is found")

        last_study = Path(root_dir) / sort_studies[-1]
        return last_study

    def is_accurate(self):
        """
        indicates if method to use is accurate by reading the uc_type in the settings file
        """
        if self._config.UC_TYPE not in self.options:
            self.logger.info(
                f"{self._config.UC_TYPE} not specified, {self._config.settings_default[self._config.UC_TYPE]} used."
            )
        uc_type = self.options.get(
            self._config.UC_TYPE, self._config.settings_default[self._config.UC_TYPE]
        )
        assert uc_type in [self._config.EXPANSION_ACCURATE,
                           self._config.EXPANSION_FAST]
        return uc_type == self._config.EXPANSION_ACCURATE

    class MissingFile(Exception):
        pass

    def is_relaxed(self):
        """
        indicates if method to use is relaxed by reading the relaxation_type
        from the settings file
        """
        if "master" not in self.options:
            self.logger.info(
                f" master options is not defined, {self._config.settings_default['master']} used"
            )

        return self.get_master_formulation() == "relaxed"

    def keep_mps(self) -> bool:
        return self._config.keep_mps

    def antares_exe(self):
        return self.exe_path(self._config.ANTARES)

    def lp_namer_exe(self):
        return self.exe_path(self._config.LP_NAMER)

    def benders_exe(self):
        return self.exe_path(self._config.BENDERS)

    def merge_mps_exe(self):
        return self.exe_path(self._config.MERGE_MPS)

    def study_update_exe(self):
        return self.exe_path(self._config.STUDY_UPDATER)

    def sensitivity_exe(self):
        return self.exe_path(self._config.SENSITIVITY_EXE)

    def full_run_exe(self):
        return self.exe_path(self._config.FULL_RUN)

    def antares_archive_updater_exe(self):
        return self.exe_path(self._config.ANTARES_ARCHIVE_UPDATER)

    def outer_loop_exe(self):
        return self.exe_path(self._config.OUTER_LOOP)

    def method(self):
        return self._config.method

    def n_mpi(self):
        return self._config.n_mpi

    def step(self):
        return self._config.step

    def simulation_name(self):
        return self._xpansion_simulation_name

    def antares_n_cpu(self):
        return self._config.antares_n_cpu

    def json_file_path(self):
        return os.path.join(self.expansion_dir(), self.json_name())

    def json_name(self):
        return self._config.JSON_NAME

    def last_iteration_json_file_path(self):
        return os.path.join(self.expansion_dir(), self.last_iteration_json_file_name())

    def last_iteration_json_file_name(self):
        return self._config.LAST_ITERATION_JSON_FILE_NAME

    def json_sensitivity_out_path(self):
        return os.path.join(self._sensitivity_dir(), self._config.JSON_SENSITIVITY_OUT)

    def structure_file_path(self):
        # Assumes that structure file is always the default, ok for now as the user cannot set it, but could it be dangerous if later we wish for some reasons modify its name to a non-default one
        return os.path.join(
            self.simulation_lp_path(
            ), self._config.options_default["STRUCTURE_FILE"]
        )

    def last_master_file_path(self):
        # The 'last_iteration' literal is only hard-coded in Worker.cpp, should we introduce a new variable in _config.options_default ?
        return os.path.join(
            self.simulation_lp_path(),
            self._config.options_default["MASTER_NAME"] +
            "_last_iteration.mps",
        )

    def last_master_basis_path(self):
        return os.path.join(
            self.simulation_lp_path(),
            self._config.options_default["MASTER_NAME"] + "_last_basis.bss",
        )

    def oversubscribe(self):
        return self._config.oversubscribe

    def allow_run_as_root(self):
        return self._config.allow_run_as_root

    def timelimit(self):
        """
        returns the timelimit read from the settings file
        :return: timelimit value or 0 if the gap is negative
        """
        timelimit_str = self.options.get(
            "timelimit", self._config.settings_default["timelimit"]
        )
        return 1e12 if timelimit_str in ("+Inf", "+infini") else int(timelimit_str)

    def log_level(self):
        """
        returns the log_level read from the settings file
        :return: log_level value
        """
        log_level_str = self.options.get(
            "log_level", self._config.settings_default["log_level"]
        )
        return int(log_level_str)

    def sensitivity_log_file(self) -> Path:
        return Path(
            os.path.join(self._sensitivity_dir(),
                         self._config.SENSITIVITY_LOG_FILE)
        )

    class MissingSimulationName(Exception):
        pass

    class InvalidSimulationName(Exception):
        pass

    class MissingAntaresOutput(Exception):
        pass

    def check_NTC_column_constraints(self, antares_version):
        checker = ChronicleChecker(self._config.data_dir, antares_version)
        checker.check_chronicle_constraints()

    def mpi_exe(self):
        return self.exe_path(Path(self._config.MPIEXEC).name)

    def outer_loop_options_path(self):
        return os.path.join(self.outer_loop_dir(), self._config.OUTER_LOOP_FILE)

    def outer_loop_dir(self):
        return os.path.normpath(os.path.join(
            self.data_dir(),
            self._config.USER,
            self._config.EXPANSION,
            self._config.OUTER_LOOP_DIR))
