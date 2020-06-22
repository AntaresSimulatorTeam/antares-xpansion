"""
    Class to control the execution of the optimization session
"""

import shutil
import configparser
import glob
import os
import subprocess
import sys

from antares_xpansion.input_checker import check_candidates_file
from antares_xpansion.input_checker import check_settings_file
from antares_xpansion.xpansion_utils import read_and_write_mps

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

        self.check_candidates()
        self.check_settings()

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
            return self.exe_path(solver) +" "+ self.config.OPTIONS_TXT
        elif solver == self.config.BENDERS_MPI:
            return self.config.MPI_LAUNCHER +" "+\
                    self.config.MPI_N + ' 1 ' + self.exe_path(solver) +" "+ self.config.OPTIONS_TXT
        #solver == self.config.BENDERS_SEQUENTIAL:
        return self.exe_path(solver) +" "+ self.config.OPTIONS_TXT

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

    def weights_file(self, filename):
        """
            returns the path to a yearly-weights file

            :param filename: name of the yearly-weights file

            :return: path to input yearly-weights file
        """
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION, filename)


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

    def launch(self):
        lp_path = self.generate_mps_files()
        if self.args.method == "mpibenders":
            # self.launch_optimization(lp_path, self.config.BENDERS_MPI)
            print("MPI not handled yet")
        elif self.args.method == "mergeMPS":
            self.launch_optimization(lp_path, self.config.MERGE_MPS)
        elif self.args.method == "sequential":
            self.launch_optimization(lp_path, self.config.BENDERS_SEQUENTIAL)
        elif self.args.method == "both":
            #TODO both ??
            print("MPI not handled yet")
        else:
            print("Illegal optim method")
            sys.exit(0)
    def check_candidates(self):
        """
            checks that candidates file has correct format
        """
        #check file existence
        if not os.path.isfile(self.candidates()):
            print('Missing file : %s was not retrieved in the indicated path : ', self.candidates())
            sys.exit(0)

        check_candidates_file(self)

    def check_settings(self):
        """
            checks that settings file has correct format
        """
        #check file existence
        if not os.path.isfile(self.settings()):
            print('Missing file : %s was not retrieved in the indicated path : ', self.settings())
            sys.exit(0)

        check_settings_file(self)

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
        old_output = os.listdir(self.antares_output())
        print([self.antares(), self.data_dir()])
        with open(self.antares() + '.log', 'w') as output_file:
            subprocess.call(self.antares() +" "+ self.data_dir(), shell=True,
                            stdout=output_file,
                            stderr=output_file)
        new_output = os.listdir(self.antares_output())
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
        area_files = glob.glob(os.path.join(output_path, 'about-the-study/areas.txt'))
        interco_files = glob.glob(os.path.join(output_path, 'about-the-study/links.txt'))
        assert len(area_files) == 1
        assert len(interco_files) == 1
        shutil.copy(area_files[0], os.path.join(output_path, 'area.txt'))
        shutil.copy(interco_files[0], os.path.join(output_path, 'interco.txt'))
        lp_path = os.path.join(output_path, 'lp')
        os.makedirs(lp_path)
        is_relaxed = 'relaxed' if self.is_relaxed() else 'integer'
        with open(self.exe_path(self.config.LP_NAMER) + '.log', 'w') as output_file:
            subprocess.call(self.exe_path(self.config.LP_NAMER) +" "+ output_path +" "+ is_relaxed,
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
        options_values["SLAVE_WEIGHT_VALUE"] = str(self.nb_years())
        options_values["GAP"] = self.optimality_gap()
        print('Number of years is {}, setting SLAVE_WEIGHT_VALUE to {} '.
              format(self.nb_years(), options_values["SLAVE_WEIGHT_VALUE"]))
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
