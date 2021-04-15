"""
    Class to control the execution of the optimization session
"""

import shutil
import configparser
import glob
import os
import subprocess
import sys

from pathlib import Path

from antares_xpansion.general_data_reader import GeneralDataIniReader, IniReader
from antares_xpansion.input_checker import check_candidates_file
from antares_xpansion.input_checker import check_settings_file
from antares_xpansion.xpansion_utils import read_and_write_mps


class XpansionDriver():
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

        self.candidates_list = []

        with open(self.settings(), 'r') as file_l:
            self.options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines() if line.strip()})

        self.check_candidates()
        self.check_settings()

        self.changed_val = {('[' + self.config.OPTIMIZATION + ']', self.config.EXPORT_MPS): 'true',
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
                            '[other preferences]', 'unit-commitment-mode'): 'accurate' if self.is_accurate() else 'fast'
                            }

        self.nb_years = GeneralDataIniReader(Path(self.general_data())).get_nb_activated_year()

    def exe_path(self, exe):
        """
            prefixes the input exe with the install direcectory containing the binaries

            :param exe: executable name

            :return: path to specified executable
        """
        return os.path.normpath(os.path.join(self.config.installDir, exe))

    def antares(self):
        """
            returns antares binaries location
        """
        return os.path.normpath(os.path.join(self.config.installDir, self.config.ANTARES))

    def general_data(self):
        """
            returns path to general data ini file
        """
        return os.path.normpath(os.path.join(self.data_dir(),
                                             self.config.SETTINGS, self.config.GENERAL_DATA_INI))

    def settings(self):
        """
            returns path to setting ini file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, self.config.SETTINGS_INI))

    def candidates(self):
        """
            returns path to candidates ini file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, self.config.CANDIDATES_INI))

    def capacity_file(self, filename):
        """
            returns path to input capacity file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, self.config.CAPADIR, filename))

    def weights_file(self, filename):
        """
            returns the path to a yearly-weights file

            :param filename: name of the yearly-weights file

            :return: path to input yearly-weights file
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, filename))

    def antares_output(self):
        """
            returns path to antares output data directory
        """
        return os.path.normpath(os.path.join(self.data_dir(), self.config.OUTPUT))

    def data_dir(self):
        """
            returns path to the data directory
        """
        return self.config.dataDir

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
        return os.path.normpath(os.path.join(self.data_dir(), self.config.USER,
                                             self.config.EXPANSION, additional_constraints_filename))

    def launch(self):
        """
            launch antares xpansion steps
        """
        self.clear_old_log()

        if self.config.step == "full":
            lp_path = self.generate_mps_files()
            self.launch_optimization(lp_path)
            self.update_step(self.simulation_name)
        elif self.config.step == "antares":
            self.pre_antares()
            self.launch_antares()
        elif self.config.step == "getnames":
            if self.config.simulationName:
                self.get_names(self.config.simulationName)
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "lp":
            if self.config.simulationName:
                self.lp_step(self.config.simulationName)
                output_path = os.path.normpath(os.path.join(self.antares_output(), self.config.simulationName))
                self.set_options(output_path)
        elif self.config.step == "update":
            if self.config.simulationName:
                self.update_step(self.config.simulationName)
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        elif self.config.step == "optim":
            if self.config.simulationName:
                lp_path = os.path.normpath(os.path.join(self.antares_output(),
                                                        self.config.simulationName, 'lp'))
                self.launch_optimization(lp_path)
            else:
                print("Missing argument simulationName")
                sys.exit(1)
        else:
            print("Launching failed")
            sys.exit(1)

    def clear_old_log(self):
        """
            clears old log files for antares and the lp_namer
        """
        if (self.config.step in ["full", "antares"]) and (os.path.isfile(self.antares() + '.log')):
            os.remove(self.antares() + '.log')
        if (self.config.step in ["full", "lp"]) \
                and (os.path.isfile(self.exe_path(self.config.LP_NAMER) + '.log')):
            os.remove(self.exe_path(self.config.LP_NAMER) + '.log')
        if (self.config.step in ["full", "update"]) \
                and (os.path.isfile(self.exe_path(self.config.STUDY_UPDATER) + '.log')):
            os.remove(self.exe_path(self.config.STUDY_UPDATER) + '.log')

    def check_candidates(self):
        """
            checks that candidates file has correct format
        """
        # check file existence
        if not os.path.isfile(self.candidates()):
            print('Missing file : %s was not retrieved.' % self.candidates())
            sys.exit(1)

        check_candidates_file(self)

    def check_settings(self):
        """
            checks that settings file has correct format
        """
        # check file existence
        if not os.path.isfile(self.settings()):
            print('Missing file : %s was not retrieved.' % self.settings())
            sys.exit(1)

        check_settings_file(self)

    def pre_antares(self):
        """
            modifies the general data file to configure antares execution
        """

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

    def _get_new_line(self, line, section, key):
        if (section, key) in self.changed_val:
            new_val = self.changed_val[(section, key)]
            if new_val:
                line = key + ' = ' + new_val + '\n'
            else:
                line = None
        return line

    def launch_antares(self):
        """
            launch antares

            :return: name of the new simulation's directory
        """
        # if not os.path.isdir(driver.antares_output()):
        #     os.mkdir(driver.antares_output(), )
        old_output = os.listdir(self.antares_output())
        returned_l = subprocess.run(self.get_antares_cmd(), shell=False,
                                        stdout= subprocess.DEVNULL,
                                        stderr= subprocess.DEVNULL)
        if returned_l.returncode != 0:
            print("WARNING: exited antares with status %d" % returned_l.returncode)
        new_output = os.listdir(self.antares_output())
        assert len(old_output) + 1 == len(new_output)
        diff = list(set(new_output) - set(old_output))
        return diff[0]

    def get_antares_cmd(self):
        return [self.antares(), self.data_dir()]

    def post_antares(self, antares_output_name):
        """
            creates necessary files for simulation using the antares simulation output files,
            the existing configuration files, get_names and the lp_namer executable

            :param antares_output_name: name of the antares simulation output directory

            :return: path to the lp output directory
        """
        output_path = os.path.normpath(os.path.join(self.antares_output(), antares_output_name))
        self.get_names(antares_output_name)
        lp_path = self.lp_step(antares_output_name)
        self.set_options(output_path)
        return lp_path

    def get_names(self, antares_output_name):
        """
            produces a .txt file describing the weekly problems:
            each line of the file contains :
             - mps file name
             - variables file name
             - constraints file name

            :param antares_output_name: name of the antares simulation output directory

            produces a file named with xpansionConfig.MPS_TXT
        """
        output_path = os.path.normpath(os.path.join(self.antares_output(), antares_output_name))
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

    def lp_step(self, antares_output_name):
        """
            copies area and interco files and launches the lp_namer

            :param antares_output_name: path to the antares simulation output directory

            produces a file named with xpansionConfig.MPS_TXT
        """
        output_path = os.path.normpath(os.path.join(self.antares_output(), antares_output_name))

        lp_path = os.path.normpath(os.path.join(output_path, 'lp'))
        if os.path.isdir(lp_path):
            shutil.rmtree(lp_path)
        os.makedirs(lp_path)

        with open(self.get_lp_namer_log_filename(lp_path), 'w') as output_file:
            returned_l = subprocess.run(self.get_lp_namer_command(output_path), shell=False,
                                        stdout=output_file,
                                        stderr=output_file)
            if returned_l.returncode != 0:
                print("ERROR: exited lpnamer with status %d" % returned_l.returncode)
                sys.exit(1)
        return lp_path

    def get_lp_namer_log_filename(self, lp_path):
        return os.path.join(lp_path , self.config.LP_NAMER + '.log')

    def get_lp_namer_command(self, output_path):
        is_relaxed = 'relaxed' if self.is_relaxed() else 'integer'
        return [self.exe_path(self.config.LP_NAMER), "-o", output_path, "-f", is_relaxed, "-e",
                self.additional_constraints()]

    def update_step(self, antares_output_name):
        """
            updates the antares study using the candidates file and the json solution output

            :param antares_output_name: path to the antares simulation output directory
        """
        output_path = os.path.normpath(os.path.join(self.antares_output(), antares_output_name))

        with open(self.get_study_updater_log_filename(output_path), 'w') as output_file:
            returned_l = subprocess.run(self.get_study_updater_command(output_path), shell=False,
                                         stdout=output_file,
                                         stderr=output_file)
            if returned_l.returncode != 0:
                print("ERROR: exited study-updater with status %d" % returned_l)
                sys.exit(1)

    def get_study_updater_log_filename(self, output_path):
        return os.path.join(output_path , self.config.STUDY_UPDATER + '.log')

    def get_study_updater_command(self, output_path):
        return [self.exe_path(self.config.STUDY_UPDATER), "-o", output_path, "-s",
                self.config.options_default["JSON_NAME"] + ".json"]

    def launch_optimization(self, lp_path):
        """
            launch the optimization of the antaresXpansion problem using the specified solver

            :param lp_path: path to the lp directory containing input files
            (c.f. generate_mps_files)
            :param solver: name of the solver to be used
            :type solver: value in [XpansionConfig.MERGE_MPS, XpansionConfig.BENDERS_MPI,
            XpansionConfig.BENDERS_SEQUENTIAL]
        """
        output_path = os.path.normpath(os.path.join(self.antares_output(), self.simulation_name))
        self.set_options(output_path)

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
                                     stdout= sys.stdout,
                                     stderr= sys.stderr)
        if returned_l.returncode != 0:
            print("ERROR: exited solver with status %d" % returned_l.returncode)
            sys.exit(1)

        os.chdir(old_cwd)

    def get_solver_cmd(self,solver):
        """
            returns a list consisting of the path to the required solver and its launching options
        """
        assert solver in [self.config.MERGE_MPS,
                          self.config.BENDERS_MPI,
                          self.config.BENDERS_SEQUENTIAL]
        if solver == self.config.BENDERS_MPI:
            return [self.config.MPI_LAUNCHER, self.config.MPI_N, str(self.config.n_mpi), self.exe_path(solver), self.config.OPTIONS_TXT]
        else:
            return [self.exe_path(solver),self.config.OPTIONS_TXT]

    def set_options(self, output_path):
        """
            generates a default option file for the solver
        """
        # computing the weight of slaves
        options_values = self.config.options_default
        options_values["SLAVE_WEIGHT_VALUE"] = str(self.nb_years)
        options_values["GAP"] = self.optimality_gap()
        options_values["MAX_ITERATIONS"] = self.max_iterations()
        # generate options file for the solver
        options_path = os.path.normpath(os.path.join(output_path, 'lp', self.config.OPTIONS_TXT))
        with open(options_path, 'w') as options_file:
            options_file.writelines(["%30s%30s\n" % (kvp[0], kvp[1])
                                     for kvp in options_values.items()])

    def generate_mps_files(self):
        """
            launches antares to produce mps files and
            sets the simulation_name attribute

            :return: path to the lp output directory
        """
        # setting antares options
        print("-- pre antares")
        self.pre_antares()
        # launching antares
        print("-- launching antares")
        self.simulation_name = self.launch_antares()
        # writting things
        print("-- post antares")
        lp_path = self.post_antares(self.simulation_name)
        return lp_path
