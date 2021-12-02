"""
    Class to work on config
"""

import os
import sys

from pathlib import Path

import re

from antares_xpansion.general_data_reader import GeneralDataIniReader
from antares_xpansion.input_checker import check_candidates_file, check_options
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.flushed_print import flushed_print


class ConfigLoader:
    """
        Class to control the execution of the optimization session
    """

    def __init__(self, config):
        """
            Initialise driver with a given antaresXpansion configuration,
            the system platform and parses the arguments
            :param config: configuration to use for the optimization
            :type config: XpansionConfig object
        """
        self.platform = sys.platform
        self._config = config
        self._set_simulation_name()
        self.candidates_list = []
        self._verify_settings_ini_file_exists()

        self.nb_active_years = GeneralDataIniReader(
            Path(self.general_data())).get_nb_activated_year()

        self.options = self._get_options_from_settings_inifile()

        self.check_candidates_file_format()
        self.check_settings_file_format()

    def _set_simulation_name(self):
        if not self._config.simulation_name:
            raise ConfigLoader.MissingSimulationName(
                "Missing argument simulationName")
        else:
            self._simulation_name = self._config.simulation_name

    def _verify_settings_ini_file_exists(self):
        if not os.path.isfile(self._get_settings_ini_filepath()):
            raise ConfigLoader.MissingFile(
                ' %s was not retrieved.' % self._get_settings_ini_filepath())

    def _get_options_from_settings_inifile(self):
        with open(self._get_settings_ini_filepath(), 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines() if line.strip()})
        return options

    def check_candidates_file_format(self):
        if not os.path.isfile(self.candidates_ini_filepath()):
            raise ConfigLoader.MissingFile(
                ' %s was not retrieved.' % self.candidates_ini_filepath())

        check_candidates_file(self)

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
        return self.options.get('yearly-weights', self._config.settings_default["yearly-weights"])

    def general_data(self):
        """
            returns path to general data ini file
        """
        return os.path.normpath(os.path.join(self.data_dir(),
                                             self._config.SETTINGS, self._config.GENERAL_DATA_INI))

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
        return os.path.normpath(os.path.join(self.data_dir(), self._config.USER,
                                             self._config.EXPANSION, filename))

    def capacity_file(self, filename):
        """
            returns path to input capacity file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self._config.USER,
                                             self._config.EXPANSION, self._config.CAPADIR, filename))

    def weights_file_path(self):
        """
            returns the path to a yearly-weights file

            :return: path to input yearly-weights file
        """

        yearly_weights_filename = self.weight_file_name()
        if yearly_weights_filename:
            return self._get_path_from_file_in_xpansion_dir(yearly_weights_filename)
        else:
            return ""

    def get_absolute_optimality_gap(self):
        """
        prints and returns the absolute optimality gap read from the settings file
        :return: gap value or 0 if the gap is negative
        """
        abs_optimality_gap_str = self.options.get(
            "optimality_gap", self.config.settings_default["optimality_gap"]
        )

        return float(abs_optimality_gap_str) if float(abs_optimality_gap_str) > 0 else 0

    def get_relative_optimality_gap(self):
        """
        prints and returns the relative optimality gap read from the settings file
        :return: gap value or 1e-12 if the value is set to a lower value than 1e-12
        """
        rel_optimality_gap_str = self.options.get(
            "relative_gap", self.config.settings_default["relative_gap"]
        )

        return (
            float(rel_optimality_gap_str)
            if float(rel_optimality_gap_str) > 1e-12
            else 1e-12
        )

    def get_max_iterations(self):
        """
            prints and returns the maximum iterations read from the settings file

            :return: max iterations value or -1 if the parameter is set to +Inf or +infini
        """
        max_iterations_str = self.options.get('max_iteration',
                                              self.config.settings_default["max_iteration"])

        return float(max_iterations_str) if (
            (max_iterations_str != '+Inf') and (max_iterations_str != '+infini')) else -1

    def additional_constraints(self):
        """
            returns path to additional constraints file
        """
        additional_constraints_filename = self.options.get("additional-constraints",
                                                           self._config.settings_default["additional-constraints"])

        if additional_constraints_filename == "":
            return ""
        return self._get_path_from_file_in_xpansion_dir(additional_constraints_filename)

    def simulation_lp_path(self):
        return self._simulation_lp_path()

    def _simulation_lp_path(self):
        lp_path = os.path.normpath(os.path.join(
            self.simulation_output_path(), 'lp'))
        return lp_path

    def _verify_additional_constraints_file(self):
        if self.options.get('additional-constraints', "") != "":
            additional_constraints_path = self.additional_constraints()
            if not os.path.isfile(additional_constraints_path):
                flushed_print('Illegal value: %s is not an existent additional-constraints file'
                              % additional_constraints_path)
                sys.exit(1)

    def _verify_solver(self):
        try:
            XpansionStudyReader.check_solver(self.options.get(
                'solver', ""), self._config.AVAILABLE_SOLVER)
        except XpansionStudyReader.BaseException as e:
            flushed_print(e)
            sys.exit(1)

    def simulation_output_path(self) -> Path:
        if (self._simulation_name == "last"):
            self._set_last_simulation_name()
        return Path(os.path.normpath(os.path.join(self.antares_output(), self._simulation_name)))

    def set_options_for_benders_solver(self):
        return self._set_options_for_benders_solver()

    def _set_options_for_benders_solver(self):
        """
            generates a default option file for the solver
        """
        # computing the weight of slaves
        options_values = self._config.options_default
        options_values["SLAVE_WEIGHT_VALUE"] = str(self.nb_active_years)
        options_values["ABSOLUTE_GAP"] = self.get_absolute_optimality_gap()
        options_values["RELATIVE_GAP"] = self.get_relative_optimality_gap()
        options_values["MAX_ITERATIONS"] = self.get_max_iterations()
        options_values["SOLVER_NAME"] = XpansionStudyReader.convert_study_solver_to_option_solver(
            self.options.get('solver', "Cbc"))
        if self.weight_file_name():
            options_values["SLAVE_WEIGHT"] = self.weight_file_name()
        # generate options file for the solver
        options_path = os.path.normpath(os.path.join(
            self._simulation_lp_path(), self._config.OPTIONS_TXT))
        with open(options_path, 'w') as options_file:
            options_file.writelines(["%30s%30s\n" % (kvp[0], kvp[1])
                                     for kvp in options_values.items()])

    def _set_last_simulation_name(self):
        """
            return last simulation name    
        """
        # Get list of all dirs only in the given directory
        list_of_dirs_filter = filter(lambda x: os.path.isdir(os.path.join(self.antares_output(), x)),
                                     os.listdir(self.antares_output()))
        # Sort list of files based on last modification time in ascending order
        list_of_dirs = sorted(list_of_dirs_filter,
                              key=lambda x: os.path.getmtime(
                                  os.path.join(self.antares_output(), x))
                              )
        self._simulation_name = list_of_dirs[-1]

    def is_accurate(self):
        """
            indicates if method to use is accurate by reading the uc_type in the settings file
        """
        uc_type = self.options.get(self._config.UC_TYPE,
                                   self._config.settings_default[self._config.UC_TYPE])
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
        relaxation_type = self.options.get('master',
                                           self._config.settings_default["master"])
        assert relaxation_type in ['integer', 'relaxed', 'full_integer']
        return relaxation_type == 'relaxed'

    def keep_mps(self) -> bool:
        return self._config.keep_mps

    def antares_exe(self):
        return self.exe_path(self._config.ANTARES)

    def lp_namer_exe(self):
        return self.exe_path(self._config.LP_NAMER)

    def benders_mpi_exe(self):
        return self.exe_path(self._config.BENDERS_MPI)

    def benders_sequential_exe(self):
        return self.exe_path(self._config.BENDERS_SEQUENTIAL)

    def merge_mps_exe(self):
        return self.exe_path(self._config.MERGE_MPS)

    def study_update_exe(self):
        return self.exe_path(self._config.STUDY_UPDATER)

    def method(self):
        return self._config.method

    def n_mpi(self):
        return self._config.n_mpi

    def step(self):
        return self._config.step

    def simulation_name(self):
        return self._simulation_name

    def antares_n_cpu(self):
        return self._config.antares_n_cpu

    def json_name(self):
        return self._config.options_default["JSON_NAME"]

    class MissingSimulationName(Exception):
        pass
