from unittest.mock import ANY, patch

import pytest
import os
from pathlib import Path
import yaml

from antares_xpansion.general_data_reader import IniReader
from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.general_data_processor import GeneralDataFileExceptions, GeneralDataProcessor
from tests.build_config_reader import get_antares_solver_path

SUBPROCESS_RUN = "antares_xpansion.antares_driver.subprocess.run"


class TestGeneralDataProcessor:

    def setup_method(self):
        self.generaldata_filename = "generaldata.ini"

    def test_with_non_existing_general_data_file(self, tmp_path):

        settings_dir = self.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        with pytest.raises(GeneralDataFileExceptions.GeneralDataFileNotFound):
            GeneralDataProcessor(settings_dir, True)

    def test_general_data_file_path(self, tmp_path):

        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        expected_path = settings_dir / self.generaldata_filename
        expected_path.touch()
        gen_data_proc = GeneralDataProcessor(settings_dir, True)
        assert gen_data_proc.general_data_ini_file == expected_path

    def test_no_changes_in_empty_file(self, tmp_path):
        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / self.generaldata_filename
        gen_data_path.touch()
        gen_data_proc = GeneralDataProcessor(settings_dir, True)

        with open(gen_data_proc.general_data_ini_file, "r") as reader:
            lines = reader.readlines()

        assert len(lines) == 0

    def test_values_change_in_general_file_accurate_mode(self, tmp_path):

        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / self.generaldata_filename

        is_accurate = True
        optimization = '[optimization]'
        general = '[general]'
        random_section = '[random_section]'

        other_preferences = '[other preferences]'
        default_val = "[general] \n" \
                      "mode = expansion\n" \
                      "[optimization] \n" \
                      "include-exportmps = false\n" \
                      "include-tc-minstablepower = false\n" \
                      "include-dayahead = NO\n" \
                      "include-usexprs = value\n" \
                      "include-inbasis = value\n" \
                      "include-outbasis = value\n" \
                      "include-trace = value\n" \
                      "[other preferences] \n" \
                      "unit-commitment-mode = fast\n" \
                      "[random_section] \n" \
                      "key1 = value1\n" \
                      "key2 = value2\n"

        gen_data_path.write_text(default_val)
        expected_val = {(optimization, 'include-exportmps'): 'true',
                        (optimization, 'include-exportstructure'): 'true',
                        (optimization, 'include-tc-minstablepower'): 'true',
                        (optimization, 'include-tc-min-ud-time'): 'true',
                        (optimization, 'include-dayahead'): 'true',
                        (general, 'mode'): 'expansion',
                        (other_preferences, 'unit-commitment-mode'): 'accurate',
                        (random_section, "key1"): "value1",
                        (random_section, "key2"): "value2"
                        }

        gen_data_proc = GeneralDataProcessor(settings_dir, is_accurate)

        gen_data_proc.change_general_data_file_to_configure_antares_execution()
        general_data_ini_file = gen_data_proc.general_data_ini_file
        self.verify_that_general_data_contains_expected_vals(
            general_data_ini_file, expected_val)

    def test_values_change_in_general_file_fast_mode(self, tmp_path):
        study_path = tmp_path
        settings_dir = study_path / "settings"
        settings_dir.mkdir()
        general_data_path = settings_dir / self.generaldata_filename

        is_accurate = False
        optimization = '[optimization]'
        general = '[general]'
        random_section = '[random_section]'
        other_preferences = '[other preferences]'

        default_val = "[general] \n" \
                      "mode = expansion\n" \
                      "[optimization] \n" \
                      "include-exportmps = false\n" \
                      "include-tc-minstablepower = false\n" \
                      "include-dayahead = NO\n" \
                      "include-usexprs = value\n" \
                      "include-inbasis = value\n" \
                      "include-outbasis = value\n" \
                      "include-trace = value\n" \
                      "[other preferences] \n" \
                      "unit-commitment-mode = dada\n" \
                      "[random_section] \n" \
                      "key1 = value1\n" \
                      "key2 = value2\n"

        general_data_path.write_text(default_val)

        expected_val = {(optimization, 'include-exportmps'): 'true',
                        (optimization, 'include-exportstructure'): 'true',
                        (optimization, 'include-tc-minstablepower'): 'false',
                        (optimization, 'include-tc-min-ud-time'): 'false',
                        (optimization, 'include-dayahead'): 'false',
                        (general, 'mode'): 'Economy',
                        (other_preferences, 'unit-commitment-mode'): 'fast',
                        (random_section, "key1"): "value1",
                        (random_section, "key2"): "value2"
                        }

        gen_data_proc = GeneralDataProcessor(settings_dir, is_accurate)
        gen_data_proc.change_general_data_file_to_configure_antares_execution()

        self.verify_that_general_data_contains_expected_vals(
            general_data_path, expected_val)

    def verify_that_general_data_contains_expected_vals(self, general_data_ini_file, expected_val):
        with open(general_data_ini_file, "r") as reader:
            current_section = ""
            lines = reader.readlines()
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    value = line.split('=')[1].strip()
                    if (current_section, key) in expected_val:
                        assert value == expected_val[(current_section, key)]
                else:
                    current_section = line.strip()

    @staticmethod
    def get_settings_dir(antares_study_path: Path):
        return antares_study_path / "settings"


class TestAntaresDriver:

    def test_antares_cmd(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin1"
        antares_driver = AntaresDriver(exe_path)
        # mock subprocess.run
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, 1)
            expected_cmd = [exe_path, study_dir, "--force-parallel", "1"]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3)

    def test_antares_cmd_force_parallel_option(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin2"
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [exe_path, study_dir,
                            "--force-parallel", str(n_cpu)]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3)

    def test_invalid_n_cpu(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin"
        n_cpu = -1
        expected_n_cpu = 1
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [exe_path, study_dir,
                            "--force-parallel", str(expected_n_cpu)]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3)

    def test_remove_log_file(self, tmp_path):
        study_dir = tmp_path
        exe_path = tmp_path
        log_file = str(exe_path) + '.log'
        log_file = Path(log_file).touch()
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [str(exe_path), study_dir,
                            "--force-parallel", str(n_cpu)]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3)

    def test_non_valid_exe_empty(self, tmp_path):
        study_dir = tmp_path
        settings_dir = study_dir / "settings"
        settings_dir.mkdir()
        antares_driver = AntaresDriver("")
        with pytest.raises(OSError):
            antares_driver.launch(study_dir, 1)

    def test_empty_study_dir(self, tmp_path):

        study_dir = tmp_path
        antares_driver = AntaresDriver(get_antares_solver_path())

        with pytest.raises(AntaresDriver.AntaresExecutionError):
            antares_driver.launch(study_dir, 1)

    def test_clean_antares_step(self, tmp_path):
        study_dir = tmp_path
        self.initialize_dummy_study_dir(study_dir)

        exe_path = Path("/Path/to/exe")
        output_dir = study_dir / "output"
        output_dir.mkdir()
        simulation_dir = output_dir / "my_simu"
        simulation_dir.mkdir()
        fnames = ["something_criterion_other.ext", "-1.mps", "-1.txt"]
        files_to_remove = [simulation_dir / fname for fname in fnames]
        for file in files_to_remove:
            file.write_text("")
            assert file.exists()

        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            antares_driver.launch(study_dir, 1)

            expected_cmd = [str(exe_path), study_dir, "--force-parallel", "1"]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3)

        for file in files_to_remove:
            assert not file.exists()

    def initialize_dummy_study_dir(self, study_dir):
        settings_dir = study_dir / "settings"
        settings_dir.mkdir()
        general_data_path = settings_dir / "generaldata.ini"
        default_val = "[general] \n" \
                      "mode = expansion\n" \
                      "[optimization] \n" \
                      "include-exportmps = false\n" \
                      "include-tc-minstablepower = false\n" \
                      "include-dayahead = NO\n" \
                      "include-usexprs = value\n" \
                      "include-inbasis = value\n" \
                      "include-outbasis = value\n" \
                      "include-trace = value\n" \
                      "[other preferences] \n" \
                      "unit-commitment-mode = dada\n" \
                      "[random_section] \n" \
                      "key1 = value1\n" \
                      "key2 = value2\n"
        general_data_path.write_text(default_val)
