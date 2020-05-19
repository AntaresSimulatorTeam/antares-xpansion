import argparse
import configparser
import glob
import os
import shutil
import subprocess
import sys

"""
step are :
- antares
- create mps.txt, area.txt and interco.txt (while be removed soon)
- lp_namer
"""


# dans settings.ini il y a plein de choses!!!
# uc-type = expansion-accurate ou expansion-fast


def read_and_write_mps(root_path):
    result = {}
    # print('#getcwd() : ', os.getcwd())
    # print('#root_path : ', root_path)
    sorted_root_dir = sorted(os.listdir(root_path))
    # print('#sorted_root_dir : ', sorted_root_dir)

    for instance in sorted_root_dir:
        if '.mps' in instance and not '-1.mps' in instance:
            buffer = instance.strip().split("-")
            year = int(buffer[1])
            week = int(buffer[2])
            if not (year, week) in result:
                result[year, week] = [instance, '', '']
    # print('#result : ', result)
    # for line in result.iteritems():
    #    print line

    for instance in sorted_root_dir:
        if 'variables' in instance:
            buffer = instance.strip().split("-")
            year = int(buffer[1])
            week = int(buffer[2])
            result[year, week][1] = instance

    for instance in sorted_root_dir:
        if 'constraints' in instance:
            buffer = instance.strip().split("-")
            year = int(buffer[1])
            week = int(buffer[2])
            result[year, week][2] = instance

    # for line in result.items():
    #     print(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + ' ')
    return result


class XpansionConfig:
    def __init__(self):
        self.MPI_LAUNCHER = "mpiexec"
        self.MPI_N = "-n"
        self.ANTARES = 'antares-7.0-solver'
        self.SETTINGS = 'settings'
        self.USER = 'user'
        self.EXPANSION = 'expansion'
        self.GENERALDATA_INI = 'generaldata.ini'
        self.NB_YEARS = 'nbyears'
        self.SETTINGS_INI = 'settings.ini'
        self.CANDIDATES_INI = 'candidates.ini'
        self.UC_TYPE = 'uc_type'
        self.EXPANSION_ACCURATE = 'expansion_accurate'
        self.EXPANSION_FAST = 'expansion_fast'
        self.OPTIMIZATION = 'optimization'
        self.EXPORTSTRUCTURE = 'include-exportstructure'
        self.EXPORTMPS = 'include-exportmps'
        self.TRACE = 'include-trace'
        self.USEXPRS = 'include-usexprs'
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
        self.parser.add_argument("--installDir", help="the directory where all binaries are located", required=True)
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


class XpansionDriver:
    def __init__(self, config):
        self.platform = sys.platform
        self.config = config
        self.args = self.config.parser.parse_args()

    def exe_path(self, exe):
        return os.path.join(self.args.installDir, exe)

    def solver_cmd(self, solver):
        assert solver in [self.config.MERGE_MPS, self.config.BENDERS_MPI, self.config.BENDERS_SEQUENTIAL]
        if solver == self.config.MERGE_MPS:
            return [self.exe_path(solver), self.config.OPTIONS_TXT]
        elif solver == self.config.BENDERS_MPI:
            return [self.config.MPI_LAUNCHER, self.config.MPI_N, '1', self.exe_path(solver), self.config.OPTIONS_TXT]
        elif solver == self.config.BENDERS_SEQUENTIAL:
            return [self.exe_path(solver), self.config.OPTIONS_TXT]

    def antares(self):
        return os.path.join(self.args.installDir, self.config.ANTARES)

    def general_data(self):
        return os.path.join(self.data_dir(), self.config.SETTINGS, self.config.GENERALDATA_INI)

    def settings(self):
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION, self.config.SETTINGS_INI)

    def candidates(self):
        return os.path.join(self.data_dir(), self.config.USER, self.config.EXPANSION, self.config.CANDIDATES_INI)

    def antares_output(self):
        return os.path.join(self.data_dir(), self.config.OUTPUT)

    def data_dir(self):
        return self.args.dataDir

    def is_accurate(self):

        with open(self.settings(), 'r') as file:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip() for line in file.readlines()})
            uc_type = options[self.config.UC_TYPE]
            assert uc_type in [self.config.EXPANSION_ACCURATE, self.config.EXPANSION_FAST]
            return uc_type == self.config.EXPANSION_ACCURATE
        assert false

    def is_relaxed(self):
        with open(self.settings(), 'r') as file:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip() for line in file.readlines()})
            relaxation_type = options['master']
            assert relaxation_type in ['integer', 'relaxed', 'full_integer']
            return relaxation_type == 'relaxed'
        assert false

    def optimality_gap(self):
        with open(self.settings(), 'r') as file:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip() for line in file.readlines()})
            optimality_gap_str = options['optimality_gap']
            assert not('%' in  optimality_gap_str)
            print('optimality_gap_str :', optimality_gap_str)
            return float(optimality_gap_str) if optimality_gap_str != '-Inf' else 0
        assert false

    def nb_years(self):
        ini_file = configparser.ConfigParser()
        ini_file.read(self.general_data())
        return float(ini_file['general']['nbyears'])

    def check_candidates(self):
        ini_file = configparser.ConfigParser()
        ini_file.read(self.candidates())
        # check that name does not have space
        for each_section in ini_file.sections():

            name = ini_file[each_section]['name'].strip()
            if ' ' in name:
                print('Error candidates name should not contain space, found in ', name)
                sys.exit(0)

    def pre_antares(self):
        ini_file = configparser.ConfigParser()
        ini_file.read(self.general_data())
        ini_file[self.config.OPTIMIZATION][self.config.EXPORTMPS] = "true"
        ini_file[self.config.OPTIMIZATION][self.config.EXPORTSTRUCTURE] = "true"
        ini_file[self.config.OPTIMIZATION][self.config.TRACE] = "false"
        ini_file[self.config.OPTIMIZATION][self.config.USEXPRS] = "false"
        ini_file.remove_option(self.config.OPTIMIZATION, self.config.USEXPRS)
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
        # if not os.path.isdir(driver.antares_output()):
        #     os.mkdir(driver.antares_output(), )
        old_output = os.listdir(driver.antares_output())
        print([self.antares(), self.data_dir()])
        with open(self.antares() + '.log', 'w') as output_file:
            subprocess.call([self.antares(), self.data_dir()], shell=True,
                            stdout=output_file,
                            stderr=output_file)
        new_output = os.listdir(driver.antares_output())
        print(old_output)
        print(new_output)
        assert len(old_output) + 1 == len(new_output)
        diff = list(set(new_output) - set(old_output))
        return diff[0]

    def post_antares(self, antares_output_name):
        output_path = os.path.join(self.antares_output(), antares_output_name)
        mps_txt = read_and_write_mps(output_path)
        # print(mps_txt)
        with open(os.path.join(output_path, self.config.MPS_TXT), 'w') as file:
            for line in mps_txt.items():
                file.write(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + '\n')
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
            subprocess.call([self.exe_path(self.config.LP_NAMER), output_path, is_relaxed], shell=True,
                            stdout=output_file,
                            stderr=output_file)
        self.set_options(output_path)
        return lp_path

    def launch_optimization(self, lp_path, solver):
        old_cwd = os.getcwd()
        os.chdir(lp_path)
        print('Current directory is now : ', os.getcwd())
        print('Launching {}, logs will be saved to {}.log'.format(solver, os.path.join(os.getcwd(), solver)))

        with open(solver + '.log', 'w') as output_file:
            subprocess.call(self.solver_cmd(solver), shell=True,
                            stdout=output_file,
                            stderr=output_file)
        os.chdir(old_cwd)

    def set_options(self, output_path):
        # computing the weight of slaves
        options_values = self.config.options_default
        options_values["SLAVE_WEIGHT_VALUE"] = str(driver.nb_years())
        options_values["GAP"] = self.optimality_gap()
        print('Number of years is {}, setting SLAVE_WEIGHT_VALUE to {} '.format(driver.nb_years(),
                                                                                options_values["SLAVE_WEIGHT_VALUE"]))
        # generate options file for the solver
        with open(os.path.join(output_path, 'lp', self.config.OPTIONS_TXT), 'w') as options_file:
            options_file.writelines(["%30s%30s\n" % (kvp[0], kvp[1]) for kvp in options_values.items()])

    def generate_mps_files(self):
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


config = XpansionConfig()
driver = XpansionDriver(config)
driver.check_candidates()

my_lp_path = driver.generate_mps_files()
# my_lp_path = 'D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco\\lp'
# driver.set_options('D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco')
driver.launch_optimization(my_lp_path, config.BENDERS_SEQUENTIAL)
# driver.launch_optimization(my_lp_path, config.MERGE_MPS)
