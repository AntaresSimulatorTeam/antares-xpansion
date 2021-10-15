"""
    Class to control the execution of the optimization session
"""

import shutil
import glob
import os
import subprocess
import sys
from datetime import datetime

from pathlib import Path

import re

from antares_xpansion.general_data_reader import GeneralDataIniReader, IniReader
from antares_xpansion.input_checker import check_candidates_file, check_options
from antares_xpansion.xpansion_utils import read_and_write_mps
from antares_xpansion.study_output_cleaner import StudyOutputCleaner
from antares_xpansion.yearly_weight_writer import YearlyWeightWriter
from antares_xpansion.xpansion_study_reader import XpansionStudyReader


class XpansionDriver:
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
        self.config = config
        self.simulation_name = self.config.simulationName

        self.candidates_list = []
        self._verify_settings_ini_file_exists()

        self.nb_active_years = GeneralDataIniReader(Path(self.general_data())).get_nb_activated_year()

        self.options = self._get_options_from_settings_inifile()

        self.check_candidates_file_format()
        self.check_settings_file_format()

    def _verify_settings_ini_file_exists(self):
        if not os.path.isfile(self._get_settings_ini_filepath()):
            print('Missing file : %s was not retrieved.' % self._get_settings_ini_filepath())
            sys.exit(1)

    def _get_options_from_settings_inifile(self):
        with open(self._get_settings_ini_filepath(), 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines() if line.strip()})
        return options

    def check_candidates_file_format(self):
        if not os.path.isfile(self.candidates_ini_filepath()):
            print('Missing file : %s was not retrieved.' % self.candidates_ini_filepath())
            sys.exit(1)

        check_candidates_file(self)

    def check_settings_file_format(self):
        check_options(self.options)
        self._verify_solver()
        self._verify_yearly_weights_consistency()
        self._verify_additional_constraints_file()

    def launch(self):
        """
            launch antares xpansion steps
        """
        self._clear_old_log()

        if self.config.step == "full":
            self._antares_step()
            print("-- post antares")
            self.get_names()
            self.lp_step()
            self.launch_optimization()
            self.update_step()
        elif self.config.step == "antares":
            self._antares_step()
        elif self.config.step == "getnames":
            if self.config.simulationName:
                self.get_names()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "lp":
            if self.config.simulationName:
                self.lp_step()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "update":
            if self.config.simulationName:
                self.update_step()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "optim":
            if self.config.simulationName:
                self.launch_optimization()
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        else:
            print("Launching failed")
            sys.exit(1)

    def _clear_old_log(self):
        if (self.config.step in ["full", "antares"]) and (os.path.isfile(self.antares() + '.log')):
            os.remove(self.antares() + '.log')
        if (self.config.step in ["full", "lp"]) \
                and (os.path.isfile(self.exe_path(self.config.LP_NAMER) + '.log')):
            os.remove(self.exe_path(self.config.LP_NAMER) + '.log')
        if (self.config.step in ["full", "update"]) \
                and (os.path.isfile(self.exe_path(self.config.STUDY_UPDATER) + '.log')):
            os.remove(self.exe_path(self.config.STUDY_UPDATER) + '.log')

    def _antares_step(self):
        self._change_general_data_file_to_configure_antares_execution()
        self.launch_antares()

    def _change_general_data_file_to_configure_antares_execution(self):
        print("-- pre antares")
        with open(self.general_data(), 'r') as reader:
            lines = reader.readlines()

        with open(self.general_data(), 'w') as writer:
            current_section = ""
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    line = self._get_new_line(line, current_section, key)
                else:
                    current_section = line.strip()

                if line:
                    writer.write(line)

    def launch_antares(self):
        print("-- launching antares")
        simulation_name = ""

        if not os.path.isdir(self.antares_output()):
            os.mkdir(self.antares_output())
        old_output = os.listdir(self.antares_output())

        start_time = datetime.now()

        returned_l = subprocess.run(self.get_antares_cmd(), shell=False,
                                    stdout=subprocess.DEVNULL,
                                    stderr=subprocess.DEVNULL)

        end_time = datetime.now()
        print('Antares simulation duration : {}'.format(end_time - start_time))

        if returned_l.returncode != 0:
            print("WARNING: exited antares with status %d" % returned_l.returncode)
        else:
            new_output = os.listdir(self.antares_output())
            assert len(old_output) + 1 == len(new_output)
            diff = list(set(new_output) - set(old_output))
            simulation_name = str(diff[0])
            StudyOutputCleaner.clean_antares_step((Path(self.antares_output()) / simulation_name))

        self.simulation_name = simulation_name

    def get_names(self):
        """
            produces a .txt file describing the weekly problems:
            each line of the file contains :
             - mps file name
             - variables file name
             - constraints file name

            produces a file named with xpansionConfig.MPS_TXT
        """
        print("-- get names")

        output_path = self.simulation_output_path()
        mps_txt = read_and_write_mps(output_path)
        with open(os.path.normpath(os.path.join(output_path, self.config.MPS_TXT)), 'w') as file_l:
            for line in mps_txt.items():
                file_l.write(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + '\n')

        glob_path = Path(output_path)
        area_files = [str(pp) for pp in glob_path.glob("area*.txt")]
        interco_files = [str(pp) for pp in glob_path.glob("interco*.txt")]
        assert len(area_files) == 1
        assert len(interco_files) == 1
        shutil.copy(area_files[0], os.path.normpath(os.path.join(output_path, 'area.txt')))
        shutil.copy(interco_files[0], os.path.normpath(os.path.join(output_path, 'interco.txt')))

    def lp_step(self):
        """
            copies area and interco files and launches the lp_namer

            produces a file named with xpansionConfig.MPS_TXT
        """
        print("-- lp")

        output_path = self.simulation_output_path()

        lp_path = self._simulation_lp_path()
        if os.path.isdir(lp_path):
            shutil.rmtree(lp_path)
        os.makedirs(lp_path)

        weight_file_name = self.weight_file_name()
        if weight_file_name:
            weight_list = XpansionStudyReader.get_years_weight_from_file(self.weights_file_path())
            YearlyWeightWriter(Path(output_path)).create_weight_file(weight_list, weight_file_name)

        with open(self.get_lp_namer_log_filename(lp_path), 'w') as output_file:

            start_time = datetime.now()
            returned_l = subprocess.run(self.get_lp_namer_command(output_path), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)

            end_time = datetime.now()
            print('Post antares step duration: {}'.format(end_time - start_time))

            if returned_l.returncode != 0:
                print("ERROR: exited lpnamer with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.config.keep_mps:
                StudyOutputCleaner.clean_lpnamer_step(Path(output_path))
        self._set_options_for_benders_solver()

    def launch_optimization(self):
        """
            launch the optimization of the antaresXpansion problem using the specified solver

            XpansionConfig.BENDERS_SEQUENTIAL]
        """
        self._set_options_for_benders_solver()
        lp_path = self._simulation_lp_path()

        old_cwd = os.getcwd()
        os.chdir(lp_path)
        print('Current directory is now : ', os.getcwd())

        solver = None
        if self.config.method == "mpibenders":
            solver = self.config.BENDERS_MPI
        elif self.config.method == "mergeMPS":
            solver = self.config.MERGE_MPS
        elif self.config.method == "sequential":
            solver = self.config.BENDERS_SEQUENTIAL
        elif self.config.method == "both":
            print('method "both" is not handled yet')
            sys.exit(1)
        else:
            print("Illegal optim method")
            sys.exit(1)

        # delete execution logs
        logfile_list = glob.glob('./' + solver + 'Log*')
        for file_path in logfile_list:
            try:
                os.remove(file_path)
            except OSError:
                print("Error while deleting file : ", file_path)
        if os.path.isfile(solver + '.log'):
            os.remove(solver + '.log')

        returned_l = subprocess.run(self.get_solver_cmd(solver), shell=False,
                                    stdout=sys.stdout,
                                    stderr=sys.stderr)
        if returned_l.returncode != 0:
            print("ERROR: exited solver with status %d" % returned_l.returncode)
            sys.exit(1)
        elif not self.config.keep_mps:
            StudyOutputCleaner.clean_benders_step(self.simulation_output_path())
        os.chdir(old_cwd)

    def update_step(self):
        """
            updates the antares study using the candidates file and the json solution output

        """
        output_path = self.simulation_output_path()

        with open(self.get_study_updater_log_filename(), 'w') as output_file:
            returned_l = subprocess.run(self.get_study_updater_command(output_path), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)
            if returned_l.returncode != 0:
                print("ERROR: exited study-updater with status %d" % returned_l.returncode)
                sys.exit(1)
            elif not self.config.keep_mps:
                StudyOutputCleaner.clean_study_update_step(output_path)

    def antares_output(self):
        """
            returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.OUTPUT))

    def get_antares_cmd(self):
        return [self.antares(), self.data_dir()]

    def antares(self):
        """
            returns antares binaries location
        """
        return self.exe_path(self.config.ANTARES)

    def exe_path(self, exe):
        """
            prefixes the input exe with the install directory containing the binaries

            :param exe: executable name

            :return: path to specified executable
        """
        return os.path.normpath(os.path.join(self.config.installDir, exe))

    def data_dir(self):
        """
            returns path to the data directory
        """
        return self.config.dataDir

    def weight_file_name(self):
        return self.options.get('yearly-weights', self.config.settings_default["yearly-weights"])

    def general_data(self):
        """
            returns path to general data ini file
        """
        return os.path.normpath(os.path.join(self.data_dir(),
                                             self.config.SETTINGS, self.config.GENERAL_DATA_INI))

    def _get_settings_ini_filepath(self):
        """
            returns path to setting ini file
        """
        return self._get_path_from_file_in_xpansion_dir(self.config.SETTINGS_INI)

    def candidates_ini_filepath(self):
        """
            returns path to candidates ini file
        """
        return self._get_path_from_file_in_xpansion_dir(self.config.CANDIDATES_INI)

    def _get_path_from_file_in_xpansion_dir(self, filename):
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, filename))

    def capacity_file(self, filename):
        """
            returns path to input capacity file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, self.config.CAPADIR, filename))

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

    def is_accurate(self):
        """
            indicates if method to use is accurate by reading the uc_type in the settings file
        """
        uc_type = self.options.get(self.config.UC_TYPE,
                                   self.config.settings_default[self.config.UC_TYPE])
        assert uc_type in [self.config.EXPANSION_ACCURATE, self.config.EXPANSION_FAST]
        return uc_type == self.config.EXPANSION_ACCURATE

    def is_relaxed(self):
        """
            indicates if method to use is relaxed by reading the relaxation_type
            from the settings file
        """
        relaxation_type = self.options.get('master',
                                           self.config.settings_default["master"])
        assert relaxation_type in ['integer', 'relaxed', 'full_integer']
        return relaxation_type == 'relaxed'

    def optimality_gap(self):
        """
            prints and returns the optimality gap read from the settings file

            :return: gap value or 0 if the gap is set to -Inf
        """
        optimality_gap_str = self.options.get('optimality_gap',
                                              self.config.settings_default["optimality_gap"])
        assert not '%' in optimality_gap_str
        return float(optimality_gap_str) if optimality_gap_str != '-Inf' else 0

    def max_iterations(self):
        """
            prints and returns the maximum iterations read from the settings file

            :return: max iterations value or -1 if the parameter is is set to +Inf
        """
        max_iterations_str = self.options.get('max_iteration',
                                              self.config.settings_default["max_iteration"])
        assert not '%' in max_iterations_str
        return float(max_iterations_str) if (
                (max_iterations_str != '+Inf') and (max_iterations_str != '+infini')) else -1

    def additional_constraints(self):
        """
            returns path to additional constraints file
        """
        additional_constraints_filename = self.options.get("additional-constraints",
                                                           self.config.settings_default["additional-constraints"])

        if additional_constraints_filename == "":
            return ""
        return self._get_path_from_file_in_xpansion_dir(additional_constraints_filename)

    def _simulation_lp_path(self):
        lp_path = os.path.normpath(os.path.join(self.simulation_output_path(), 'lp'))
        return lp_path

    def _verify_yearly_weights_consistency(self):
        if self.weight_file_name():
            try:
                XpansionStudyReader.check_weights_file(self.weights_file_path(), self.nb_active_years)
            except XpansionStudyReader.BaseException as e:
                print(e)
                sys.exit(1)

    def _verify_additional_constraints_file(self):
        if self.options.get('additional-constraints', "") != "":
            additional_constraints_path = self.additional_constraints()
            if not os.path.isfile(additional_constraints_path):
                print('Illegal value: %s is not an existent additional-constraints file'
                      % additional_constraints_path)
                sys.exit(1)

    def _verify_solver(self):
        try:
            XpansionStudyReader.check_solver(self.options.get('solver', ""), self.config)
        except XpansionStudyReader.BaseException as e:
            print(e)
            sys.exit(1)

    def _get_new_line(self, line, section, key):
        changed_val = self._get_values_to_change_general_data_file()
        if (section, key) in changed_val:
            new_val = changed_val[(section, key)]
            if new_val:
                line = key + ' = ' + new_val + '\n'
            else:
                line = None
        return line

    def _get_values_to_change_general_data_file(self):
        return {('[' + self.config.OPTIMIZATION + ']', self.config.EXPORT_MPS): 'true',
                ('[' + self.config.OPTIMIZATION + ']', self.config.EXPORT_STRUCTURE): 'true',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-tc-minstablepower'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-tc-min-ud-time'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']',
                 'include-dayahead'): 'true' if self.is_accurate() else 'false',
                ('[' + self.config.OPTIMIZATION + ']', self.config.USE_XPRS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.INBASIS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.OUTBASIS): None,
                ('[' + self.config.OPTIMIZATION + ']', self.config.TRACE): None,
                ('[general]', 'mode'): 'expansion' if self.is_accurate() else 'Economy',
                (
                    '[other preferences]',
                    'unit-commitment-mode'): 'accurate' if self.is_accurate() else 'fast'
                }

    def simulation_output_path(self) -> Path:
        if (self.simulation_name == "last"):
            self._set_last_simulation_name()
        return Path(os.path.normpath(os.path.join(self.antares_output(), self.simulation_name)))

    def get_lp_namer_log_filename(self, lp_path):
        return os.path.join(lp_path, self.config.LP_NAMER + '.log')

    def get_lp_namer_command(self, output_path):
        is_relaxed = 'relaxed' if self.is_relaxed() else 'integer'
        return [self.exe_path(self.config.LP_NAMER), "-o", output_path, "-f", is_relaxed, "-e",
                self.additional_constraints()]

    def get_study_updater_log_filename(self):
        return os.path.join(self.simulation_output_path(), self.config.STUDY_UPDATER + '.log')

    def get_study_updater_command(self, output_path):
        return [self.exe_path(self.config.STUDY_UPDATER), "-o", output_path, "-s",
                self.config.options_default["JSON_NAME"] + ".json"]

    def get_solver_cmd(self, solver):
        """
            returns a list consisting of the path to the required solver and its launching options
        """
        assert solver in [self.config.MERGE_MPS,
                          self.config.BENDERS_MPI,
                          self.config.BENDERS_SEQUENTIAL]
        if solver == self.config.BENDERS_MPI:
            return [self.config.MPI_LAUNCHER, self.config.MPI_N, str(self.config.n_mpi), self.exe_path(solver),
                    self.config.OPTIONS_TXT]
        else:
            return [self.exe_path(solver), self.config.OPTIONS_TXT]

    def _set_options_for_benders_solver(self):
        """
            generates a default option file for the solver
        """
        # computing the weight of slaves
        options_values = self.config.options_default
        options_values["SLAVE_WEIGHT_VALUE"] = str(self.nb_active_years)
        options_values["GAP"] = self.optimality_gap()
        options_values["MAX_ITERATIONS"] = self.max_iterations()
        options_values["SOLVER_NAME"] = XpansionStudyReader.convert_study_solver_to_option_solver(
            self.options.get('solver', "Cbc"))
        if self.weight_file_name():
            options_values["SLAVE_WEIGHT"] = self.weight_file_name()
        # generate options file for the solver
        options_path = os.path.normpath(os.path.join(self._simulation_lp_path(), self.config.OPTIONS_TXT))
        with open(options_path, 'w') as options_file:
            options_file.writelines(["%30s%30s\n" % (kvp[0], kvp[1])
                                     for kvp in options_values.items()])

    def _set_last_simulation_name(self):
        """
            return last simulation name    
        """

        # simulation name folder YYYYMMDD-HHMMeco
        classic_simulation_name_regex = re.compile(
            "^\d{4}(0[1-9]|1[0-2])(0[1-9]|[12][0-9]|3[01])-([0-1]?[0-9]|2[0-3])[0-5][0-9]eco$")

        simulations_list = []
        for file in os.listdir(self.antares_output()):
            if (os.path.isdir(os.path.normpath(os.path.join(self.antares_output(), file))) and re.fullmatch(
                    classic_simulation_name_regex, file)):
                simulations_list.append(file)
        sorted_simulations_list = sorted(simulations_list)
        assert len(sorted_simulations_list) != 0
        self.simulation_name = sorted_simulations_list[-1]
        
