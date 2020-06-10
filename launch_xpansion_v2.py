"""
step are :
       - antares
       - create mps.txt, area.txt and interco.txt (while be removed soon)
       - lp_namer
"""

import shutil
import argparse
import configparser
import glob
import os
import shutil
import subprocess
import sys
from sets import Set

# dans settings.ini il y a plein de choses!!!
# uc-type = expansion-accurate ou expansion-fast


def read_and_write_mps(root_path):
    """
        :return: a dictionary giving instance file, variables file and
        constraints file per (year, week)
    """
    result = {}
    # print('#getcwd() : ', os.getcwd())
    # print('#root_path : ', root_path)
    sorted_root_dir = sorted(os.listdir(root_path))
    # print('#sorted_root_dir : ', sorted_root_dir)

    for instance in sorted_root_dir:
        if '.mps' in instance and not '-1.mps' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            if (year, week) not in result:
                result[year, week] = [instance, '', '']
    # print('#result : ', result)
    # for line in result.iteritems():
    #    print line

    for instance in sorted_root_dir:
        if 'variables' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            result[year, week][1] = instance

    for instance in sorted_root_dir:
        if 'constraints' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            result[year, week][2] = instance

    # for line in result.items():
    #     print(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + ' ')
    return result


class XpansionConfig(object):
    """
        Class defininf the parameters for an AntaresXpansion session
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods

    def __init__(self):
        self.MPI_LAUNCHER = "mpiexec"
        self.MPI_N = "-n"
        self.ANTARES = 'antares-7.0-solver'
        self.SETTINGS = 'settings'
        self.USER = 'user'
        self.EXPANSION = 'expansion'
        self.CAPADIR = 'capa'
        self.GENERAL_DATA_INI = 'generaldata.ini'
        self.NB_YEARS = 'nbyears'
        self.SETTINGS_INI = 'settings.ini'
        self.CANDIDATES_INI = 'candidates.ini'
        self.UC_TYPE = 'uc_type'
        self.EXPANSION_ACCURATE = 'expansion_accurate'
        self.EXPANSION_FAST = 'expansion_fast'
        self.OPTIMIZATION = 'optimization'
        self.EXPORT_STRUCTURE = 'include-exportstructure'
        self.EXPORT_MPS = 'include-exportmps'
        self.TRACE = 'include-trace'
        self.USE_XPRS = 'include-usexprs'
        self.INBASIS = 'include-inbasis'
        self.OUTBASIS = 'include-outbasis'

        self.OUTPUT = 'output'
        self.OPTIONS_TXT = 'options.txt'
        self.MERGE_MPS = "merge_mps"
        self.MPS_TXT = "mps.txt"
        self.BENDERS_MPI = "bendersmpi"
        self.BENDERS_SEQUENTIAL = "benderssequential"
        self.LP_NAMER = "lp_namer"

        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--step", choices=["lp", "optim", "full", "antares", "getnames"],
                                 help="generate the lp")
        self.parser.add_argument("--dataDir", help="antares study data directory", required=True)
        self.parser.add_argument("--installDir",
                                 help="the directory where all binaries are located", required=True)
        self.parser.add_argument("--method", type=str, choices=["mpibenders", "mergeMPS", "both"],
                                 help="choose the optimization method")

        self.options_default = {
            'LOG_LEVEL': '3',
            'MAX_ITERATIONS': '-1',
            'GAP': '1e-06',
            'AGGREGATION': '0',
            'OUTPUTROOT': '.',
            'TRACE': '0',
            'DELETE_CUT': '0',
            'LOG_OUTPUT': 'COMMAND',
            'SLAVE_WEIGHT': 'CONSTANT',
            'SLAVE_WEIGHT_VALUE': '1',
            'MASTER_NAME': 'master',
            'SLAVE_NUMBER': '-1',
            'STRUCTURE_FILE': 'structure.txt',
            'INPUTROOT': '.',
            'BASIS': '1',
            'ACTIVECUTS': '0',
            'THRESHOLD_AGGREGATION': '0',
            'THRESHOLD_ITERATION': '0',
            'RAND_AGGREGATION': '0',
            'MASTER_METHOD': 'SIMPLEX',
            'CSV_NAME': 'benders_output_trace',
            'BOUND_ALPHA': '1',
            'XPRESS_TRACE': '0',
        }


class XpansionDriver(object):
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
        self.args = self.config.parser.parse_args()

    def exe_path(self, exe):
        """
            prefiwex the input exe with the install direcectory containing the binaries

            :param exe: executable name

            :return: path to specified executable
        """
        return os.path.join(self.args.installDir, exe)

    def solver_cmd(self, solver):
        """
            returns a list consisting of the path to the required solver and its launching options
        """
        assert solver in [self.config.MERGE_MPS,
                          self.config.BENDERS_MPI,
                          self.config.BENDERS_SEQUENTIAL]
        if solver == self.config.MERGE_MPS:
            return [self.exe_path(solver), self.config.OPTIONS_TXT]
        elif solver == self.config.BENDERS_MPI:
            return [self.config.MPI_LAUNCHER,
                    self.config.MPI_N, '1', self.exe_path(solver), self.config.OPTIONS_TXT]
        #solver == self.config.BENDERS_SEQUENTIAL:
        return [self.exe_path(solver), self.config.OPTIONS_TXT]

    def antares(self):
        """
            returns antares binaries location
        """
        return os.path.join(self.args.installDir, self.config.ANTARES)

    def general_data(self):
        """
            returns path to general data ini file
        """
        return os.path.join(self.data_dir(), self.config.SETTINGS, self.config.GENERAL_DATA_INI)

    def settings(self):
        """
            returns path to setting ini file
        """
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION,
                            self.config.SETTINGS_INI)

    def candidates(self):
        """
            returns path to candidates ini file
        """
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION,
                            self.config.CANDIDATES_INI)

    def capacity_file(self, filename):
        """
            returns path to input capacity file
        """
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION,
                            self.config.CAPADIR, filename)


    def antares_output(self):
        """
            returns path to antares output data directory
        """
        return os.path.join(self.data_dir(), self.config.OUTPUT)

    def data_dir(self):
        """
            returns path to the data directory
        """
        return self.args.dataDir

    def is_accurate(self):
        """
            indicates if method to use is accurate by reading the uc_type in the settings file
        """

        with open(self.settings(), 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines()})
            uc_type = options[self.config.UC_TYPE]
            assert uc_type in [self.config.EXPANSION_ACCURATE, self.config.EXPANSION_FAST]
            return uc_type == self.config.EXPANSION_ACCURATE
        assert False

    def is_relaxed(self):
        """
            indicates if method to use is relaxed by reading the relaxation_type
            from the settings file
        """
        with open(self.settings(), 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines()})
            relaxation_type = options['master']
            assert relaxation_type in ['integer', 'relaxed', 'full_integer']
            return relaxation_type == 'relaxed'
        assert False

    def optimality_gap(self):
        """
            prints and returns the optimality gap read from the settings file

            :return: gap value or 0 if the gap is set to -Inf
        """
        with open(self.settings(), 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines()})
            optimality_gap_str = options['optimality_gap']
            assert not '%' in  optimality_gap_str
            print('optimality_gap_str :', optimality_gap_str)
            return float(optimality_gap_str) if optimality_gap_str != '-Inf' else 0
        assert False

    def nb_years(self):
        """
            returns the nubyears parameter value read from the general data file
        """
        ini_file = configparser.ConfigParser()
        ini_file.read(self.general_data())
        return float(ini_file['general']['nbyears'])

    def check_candidate_option_type(self, option, value):
        options_types = {   'name' : 'string',
                            'enable' : 'string',
                            'candidate-type' : 'string',
                            'investment-type' : 'string',
                            'link' : 'string',
                            'annual-cost-per-mw' : 'non-negative',
                            'unit-size' : 'non-negative',
                            'max-units' : 'non-negative',
                            'max-investment' : 'non-negative',
                            'relaxed' : 'string',
                            'has-link-profile' : 'string',
                            'has-link-profile-indirect' : 'string',
                            'link-profile' : 'string',
                            'link-profile-indirect' : 'string',
                            'already-installed-capacity' : 'non-negative',
                            'already-installed-link-profile' : 'string',
                            'already-installed-link-profile-indirect' : 'string' }
        option_type = options_types.get(option)
        if option_type is None :
            print('check_candidate_option_type: %s option not recognized in candidates file.' % option)
            sys.exit(0)
        else :
            if option_type == 'string':
                return True
            elif option_type == 'numeric':
                return value.isnumeric()
            elif option_type == 'non-negative':
                try:
                    return (float(value) >= 0)
                except ValueError:
                    return False
            else:
                print('check_candidate_option_type: Non handled data type %s for option %s' % (option_type, option))
                sys.exit(0)

    def check_candidate_option_value(self, option, value):
        #TODO construct list from links directory by iterating on directories that have .txt files within them
        antares_links_list = None
        options_legal_values = {'name' : None,
                                'enable' : ["true", "false"],
                                'candidate-type' : ["investment", "decommissioning"],
                                'investment-type' : None,
                                'link' : antares_links_list,
                                'annual-cost-per-mw' : None,
                                'unit-size' : None,
                                'max-units' : None,
                                'max-investment' : None,
                                'relaxed' : ["true", "false"],
                                'has-link-profile' : ["true", "false"],
                                'has-link-profile-indirect' : ["true", "false"],
                                'link-profile' : None, #path exists study/user/expansion/capa/file
                                'link-profile-indirect' : None, #path exists study/user/expansion/capa/file
                                'already-installed-capacity' : None, #path exists study/user/expansion/capa/file
                                'already-installed-link-profile' : None, #path exists study/user/expansion/capa/file
                                'already-installed-link-profile-indirect' : None } #path exists study/user/expansion/capa/file
        legal_values = options_legal_values.get(option)
        if legal_values is None :
            return True
        elif value.lower() in legal_values:
            return True
        else:
            print('check_candidate_option_value: Illegal value %s for option %s allowed values are : %s' % (value, option, legal_values))
            sys.exit(0)

    def check_profile_file(self, filename):
        #check file existence
        if not os.path.isfile(self.capacity_file(filename)) :
            print('Illegal value : option can be 0, 1 or an existent filename. %s is not an existent file'
                    % self.capacity_file(filename))
            sys.exit(0)

        profile_column = []
        with open(self.capacity_file(filename), 'r') as profile_file:
            for idx, line in enumerate(profile_file):
                try:
                    line_value = float(line.strip())
                    profile_column.append(line_value)
                except ValueError:
                    print('Line %d in file %s is not a single non-negative value'
                            % (idx+1, self.capacity_file(filename)))
                    sys.exit(0)
                if line_value < 0:
                    print('Line %d in file %s indicates a negative value'
                        % (idx+1, self.capacity_file(filename)))
                    sys.exit(0)

        if len(profile_column) != 8760:
            print('file %s does not have 8760 lines'
                    % (idx+1, self.capacity_file(filename)))
            sys.exit(0)

        if any(profile_column) :
            return True
        else: #profile is 0
            return False

    def check_candidates(self):
        """
            checks that candidates file has correct format
        """
        #check file existence
        if not os.path.isfile(self.candidates()):
            print('Missing file : candidates.ini was not retrieved in the indicated path : ', self.candidates())
            sys.exit(0)

        config_changed = False

        #@TODO move to checkker class when reorganizing source code
        default_values = {  'name' : 'NA',
                            'enable' : 'true',
                            'candidate-type' : 'investment',
                            'investment-type' : 'generation',
                            'link' : 'NA',
                            'annual-cost-per-mw' : '0',
                            'unit-size' : '0',
                            'max-units' : '0',
                            'max-investment' : '0',
                            'Relaxed' : 'false',
                            'has-link-profile' : 'false',
                            'has-link-profile-indirect' : 'false',
                            'link-profile' : '1',
                            'link-profile-indirect' : '1',
                            'already-installed-capacity' : '0',
                            'already-installed-link-profile' : '1',
                            'already-installed-link-profile-indirect' : '1'}
        ini_file = configparser.ConfigParser(default_values)
        ini_file.read(self.candidates())

        # check that name is not empty and does not have space
        # check that link is not empty
        for each_section in ini_file.sections():
            name = ini_file[each_section]['name'].strip()
            link = ini_file[each_section]['link'].strip()
            if (not name) or (name == "NA") :
                print('Error candidates name cannot be empty : found in section %s' % each_section)
                sys.exit(0)
            if ' ' in name:
                print('Error candidates name should not contain space, found in section %s in "%s"'
                        % (each_section, name))
                sys.exit(0)
            if (not link) or (link == "NA") :
                print('Error candidates link cannot be empty : found in section %s' % each_section)
                sys.exit(0)

        # check some attributes unicity : name and links
        unique_attributes = ["name", "link"]
        for verified_attribute in unique_attributes:
            unique_values = Set()
            for each_section in ini_file.sections():
                value = ini_file[each_section][verified_attribute].strip()
                if value in unique_values:
                    print('Error candidates %ss have to be unique, duplicate %s %s in section %s'
                            % (verified_attribute, verified_attribute, value, each_section))
                    sys.exit(0)
                else:
                    unique_values.add(value)

        #check attributes types and values
        for each_section in ini_file.sections():
            for (option, value) in ini_file.items(each_section):
                if not self.check_candidate_option_type(option, value):
                    print("value %s for option %s has the wrong type!" % (value, option))
                    sys.exit(0)
                self.check_candidate_option_value(option, value)

        #check exclusion between max-investment and (max-units, unit-size) attributes
        for each_section in ini_file.sections():
            max_invest = float(ini_file[each_section]['max-investment'].strip())
            unit_size = float(ini_file[each_section]['unit-size'].strip())
            max_units = float(ini_file[each_section]['max-units'].strip())
            if max_invest != 0 :
                if max_units != 0 or unit_size != 0 :
                    print("Illegal values in section %s : cannot assign non-null values to max-investment and (unit-size or max_units) at the same time" % (each_section))
                    sys.exit(0)
            elif max_units == 0 or unit_size == 0 :
                print("Illegal values in section %s : need to assign non-null values to max-investment or (unit-size and max_units)" % (each_section))
                sys.exit(0)

        #check attributes profile is 0, 1 or an existent filename
        profile_attributes = ['link-profile', 'link-profile-indirect',
                                'already-installed-link-profile', 'already-installed-link-profile-indirect']
        for each_section in ini_file.sections():
            has_a_profile = False
            for attribute in profile_attributes:
                value = ini_file[each_section][attribute].strip()
                if value == '0' :
                    continue
                elif value == '1':
                    has_a_profile = True
                else :
                    has_a_profile = has_a_profile or self.check_profile_file(value)
            if not has_a_profile:
                #remove candidate if it has no profile
                print("candidate %s will be removed!" % ini_file[each_section]["name"])
                ini_file.remove_section(each_section)
                config_changed = True

        if config_changed :
            shutil.copyfile(self.candidates(), self.candidates()+".bak")
            with open(self.candidates(), 'w') as out_file:
                ini_file.write(out_file)
            print("%s file was overwritten! backup file %s created" % ( self.candidates(), self.candidates()+".bak"))


    def pre_antares(self):
        """
            modifies the general data file to configure antares execution
        """
        ini_file = configparser.ConfigParser()
        ini_file.read(self.general_data())
        ini_file[self.config.OPTIMIZATION][self.config.EXPORT_MPS] = "true"
        ini_file[self.config.OPTIMIZATION][self.config.EXPORT_STRUCTURE] = "true"
        ini_file[self.config.OPTIMIZATION][self.config.TRACE] = "false"
        ini_file[self.config.OPTIMIZATION][self.config.USE_XPRS] = "false"
        ini_file.remove_option(self.config.OPTIMIZATION, self.config.USE_XPRS)
        ini_file.remove_option(self.config.OPTIMIZATION, self.config.INBASIS)
        ini_file.remove_option(self.config.OPTIMIZATION, self.config.OUTBASIS)
        if self.is_accurate():
            ini_file['general']['mode'] = 'expansion'
            ini_file['other preferences']['unit-commitment-mode'] = 'accurate'
            ini_file[self.config.OPTIMIZATION]['include-tc-minstablepower'] = 'true'
            ini_file[self.config.OPTIMIZATION]['include-tc-min-ud-time'] = 'true'
            ini_file[self.config.OPTIMIZATION]['include-dayahead'] = 'true'
        else:
            ini_file['general']['mode'] = 'Economy'
            ini_file['other preferences']['unit-commitment-mode'] = 'fast'
            ini_file[self.config.OPTIMIZATION]['include-tc-minstablepower'] = 'false'
            ini_file[self.config.OPTIMIZATION]['include-tc-min-ud-time'] = 'false'
            ini_file[self.config.OPTIMIZATION]['include-dayahead'] = 'false'

        with open(self.general_data(), 'w') as out_file:
            ini_file.write(out_file)

    def launch_antares(self):
        """
            launch antares

            :return: name of the new simulation's directory
        """
        # if not os.path.isdir(driver.antares_output()):
        #     os.mkdir(driver.antares_output(), )
        old_output = os.listdir(DRIVER.antares_output())
        print([self.antares(), self.data_dir()])
        with open(self.antares() + '.log', 'w') as output_file:
            subprocess.call([self.antares(), self.data_dir()], shell=True,
                            stdout=output_file,
                            stderr=output_file)
        new_output = os.listdir(DRIVER.antares_output())
        print(old_output)
        print(new_output)
        assert len(old_output) + 1 == len(new_output)
        diff = list(set(new_output) - set(old_output))
        return diff[0]

    def post_antares(self, antares_output_name):
        """
            creates necessary files for simulation using the antares simulation output files,
            the existing configuration files and the lpnamer executable

            :return: path to lp directory
        """
        output_path = os.path.join(self.antares_output(), antares_output_name)
        mps_txt = read_and_write_mps(output_path)
        # print(mps_txt)
        with open(os.path.join(output_path, self.config.MPS_TXT), 'w') as file_l:
            for line in mps_txt.items():
                file_l.write(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + '\n')
        area_files = glob.glob(os.path.join(output_path, 'area*.txt'))
        interco_files = glob.glob(os.path.join(output_path, 'interco*.txt'))
        assert len(area_files) == 1
        assert len(interco_files) == 1
        shutil.copy(area_files[0], os.path.join(output_path, 'area.txt'))
        shutil.copy(interco_files[0], os.path.join(output_path, 'interco.txt'))
        lp_path = os.path.join(output_path, 'lp')
        os.makedirs(lp_path)
        is_relaxed = 'relaxed' if self.is_relaxed() else 'integer'
        with open(self.exe_path(self.config.LP_NAMER) + '.log', 'w') as output_file:
            subprocess.call([self.exe_path(self.config.LP_NAMER), output_path, is_relaxed],
                            shell=True,
                            stdout=output_file,
                            stderr=output_file)
        self.set_options(output_path)
        return lp_path

    def launch_optimization(self, lp_path, solver):
        """
            launch the optimization of the antaresXpansion problem using the specified solver

            :param lp_path: path to the lp directory containing input files
            (c.f. generate_mps_files)
            :param solver: name of the solver to be used
            :type solver: value in [XpansionConfig.MERGE_MPS, XpansionConfig.BENDERS_MPI,
            XpansionConfig.BENDERS_SEQUENTIAL]
        """
        old_cwd = os.getcwd()
        os.chdir(lp_path)
        print('Current directory is now : ', os.getcwd())
        print('Launching {}, logs will be saved to {}.log'.format(solver,
                                                                  os.path.join(os.getcwd(),
                                                                               solver)))

        with open(solver + '.log', 'w') as output_file:
            subprocess.call(self.solver_cmd(solver), shell=True,
                            stdout=output_file,
                            stderr=output_file)
        os.chdir(old_cwd)

    def set_options(self, output_path):
        """
            generates a default option file for the solver
        """
        # computing the weight of slaves
        options_values = self.config.options_default
        options_values["SLAVE_WEIGHT_VALUE"] = str(DRIVER.nb_years())
        options_values["GAP"] = self.optimality_gap()
        print('Number of years is {}, setting SLAVE_WEIGHT_VALUE to {} '.
              format(DRIVER.nb_years(), options_values["SLAVE_WEIGHT_VALUE"]))
        # generate options file for the solver
        with open(os.path.join(output_path, 'lp', self.config.OPTIONS_TXT), 'w') as options_file:
            options_file.writelines(["%30s%30s\n" % (kvp[0], kvp[1])
                                     for kvp in options_values.items()])

    def generate_mps_files(self):
        """
            launches antares to produce mps files
        """
        print("starting mps generation")
        # setting antares options
        print("-- pre antares")
        self.pre_antares()
        # launching antares
        print("-- launching antares")
        antares_output_name = self.launch_antares()
        # writting things
        print("-- post antares")
        lp_path = self.post_antares(antares_output_name)
        return lp_path


CONFIG = XpansionConfig()
DRIVER = XpansionDriver(CONFIG)
DRIVER.check_candidates()

MY_LP_PATH = DRIVER.generate_mps_files()
# my_lp_path = 'D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco\\lp'
# DRIVER.set_options('D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco')
DRIVER.launch_optimization(MY_LP_PATH, CONFIG.BENDERS_SEQUENTIAL)
# DRIVER.launch_optimization(MY_LP_PATH, CONFIG.MERGE_MPS)
