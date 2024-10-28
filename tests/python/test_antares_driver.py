import configparser
import os
from pathlib import Path
from unittest.mock import patch
import pytest
from packaging import version

from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.general_data_processor import (
    GeneralDataFileExceptions,
    GeneralDataProcessor,
)
from antares_xpansion.general_data_reader import IniReader, GeneralDataIniReader

from tests.build_config_reader import get_antares_solver_path
from antares_xpansion.__version__ import __antares_simulator_version__

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

    def test_preserve_playlist(self, tmp_path):
        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / self.generaldata_filename

        with open(gen_data_path, "w") as writer:
            writer.write("[general]\nnbyears=2\nuser-playlist = true\n")
            writer.write("[playlist]\n")
            writer.write("dummy_entry = value\n")
            writer.write("playlist_year + = 1\n")
            writer.write("playlist_year + = 42\n")
            writer.write("playlist_year - = 5\n")
            writer.write("playlist_year - = 0\n")

        gen_data_proc = GeneralDataProcessor(settings_dir, True)

        gen_data_proc.change_general_data_file_to_configure_antares_execution()
        general_data_ini_file = gen_data_proc.general_data_ini_file

        xpansion_ini_reader = GeneralDataIniReader(general_data_ini_file)
        xpansion_ini_reader.get_active_years()
        config_reader = configparser.ConfigParser(strict=False)
        config_reader.read(gen_data_path)
        active_years, inactive_years = xpansion_ini_reader.get_raw_playlist()
        assert config_reader.get("playlist", "dummy_entry") == "value"
        assert active_years == [1, 42]
        assert inactive_years == [5, 0]

    def test_empty_file_should_be_populated_by_default_values(self, tmp_path):
        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / self.generaldata_filename
        gen_data_path.touch()
        gen_data_proc = GeneralDataProcessor(settings_dir, True)
        gen_data_proc.change_general_data_file_to_configure_antares_execution()

        with open(gen_data_proc.general_data_ini_file, "r") as reader:
            lines = reader.readlines()

        assert len(lines) != 0
        parser = configparser.ConfigParser()
        parser.read(gen_data_proc.general_data_ini_file)
        for option in gen_data_proc._get_values_to_change_general_data_file():
            assert parser.get(option[0], option[1]) is not None

    def test_values_change_in_general_file_accurate_mode(self, tmp_path):

        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / self.generaldata_filename

        is_accurate = True
        default_val = (
            "[general] \n"
            "mode = unrelevant\n"
            "year-by-year = true\n"
            "[optimization] \n"
            "include-exportmps = true\n"
            "include-tc-minstablepower = false\n"
            "include-dayahead = NO\n"
            "include-usexprs = value\n"
            "include-inbasis = value\n"
            "include-outbasis = value\n"
            "include-trace = value\n"
            "[output]\n"
            "storenewset = false\n"
            "[other preferences] \n"
            "unit-commitment-mode = fast\n"
            "[random_section] \n"
            "key1 = value1\n"
            "key2 = value2\n"
            "[input]\n"
            "import = blabla\n"

        )

        gen_data_path.write_text(default_val)
        optimization = "optimization"
        general = "general"
        random_section = "random_section"
        output = "output"
        other_preferences = "other preferences"

        expected_val = {
            (optimization, "include-exportmps"): "optim-1",
            (optimization, "include-exportstructure"): "true",
            (optimization, "include-tc-minstablepower"): "true",
            (optimization, "include-tc-min-ud-time"): "true",
            (optimization, "include-dayahead"): "true",
            (general, "mode"): "expansion",
            (output, "storenewset"): "true",
            (other_preferences, "unit-commitment-mode"): "accurate",
            (random_section, "key1"): "value1",
            (random_section, "key2"): "value2",
            ("adequacy patch", "include-adq-patch"): "false",
            (general, "year-by-year"): "false",
            (output, "synthesis"): "false",
            ("input", "import"): "",
        }

        gen_data_proc = GeneralDataProcessor(settings_dir, is_accurate)

        gen_data_proc.change_general_data_file_to_configure_antares_execution()
        general_data_ini_file = gen_data_proc.general_data_ini_file
        self.verify_that_general_data_contains_expected_vals(
            general_data_ini_file, expected_val
        )

    def test_values_change_in_general_file_fast_mode(self, tmp_path):
        study_path = tmp_path
        settings_dir = study_path / "settings"
        settings_dir.mkdir()
        general_data_path = settings_dir / self.generaldata_filename

        is_accurate = False
        default_val = (
            "[general] \n"
            "mode = expansion\n"
            "[optimization] \n"
            "include-exportmps = false\n"
            "include-tc-minstablepower = false\n"
            "include-dayahead = NO\n"
            "include-usexprs = value\n"
            "include-inbasis = value\n"
            "include-outbasis = value\n"
            "include-trace = value\n"
            "[output]\n"
            "storenewset = false\n"
            "synthesis = true\n"
            "[other preferences] \n"
            "unit-commitment-mode = dada\n"
            "[random_section] \n"
            "key1 = value1\n"
            "key2 = value2\n"
            "[input]\n"
            "import =\n"
        )

        general_data_path.write_text(default_val)

        # Removing '[' and ']' from sections name
        optimization = "optimization"
        general = "general"
        random_section = "random_section"
        other_preferences = "other preferences"
        output = "output"
        expected_val = {
            (optimization, "include-exportmps"): "optim-1",
            (optimization, "include-exportstructure"): "true",
            (optimization, "include-tc-minstablepower"): "false",
            (optimization, "include-tc-min-ud-time"): "false",
            (optimization, "include-dayahead"): "false",
            (general, "mode"): "Economy",
            (output, "storenewset"): "true",
            (other_preferences, "unit-commitment-mode"): "fast",
            (random_section, "key1"): "value1",
            (random_section, "key2"): "value2",
            ("adequacy patch", "include-adq-patch"): "false",
            (general, "year-by-year"): "false",
            (output, "synthesis"): "false",
            ("input", "import"): "",
        }

        gen_data_proc = GeneralDataProcessor(settings_dir, is_accurate)
        gen_data_proc.change_general_data_file_to_configure_antares_execution()

        self.verify_that_general_data_contains_expected_vals(
            general_data_path, expected_val
        )

    def verify_that_general_data_contains_expected_vals(
            self, general_data_ini_file, expected_val
    ):
        actual_config = configparser.ConfigParser()
        actual_config.read(general_data_ini_file)
        for (section, key) in expected_val:
            value = actual_config.get(section, key, fallback=None)
            assert value is not None
            print(
                f"Section {section}, key {key}, value {value}, expected {expected_val[(section, key)]}")
            assert value == expected_val[(section, key)]

        with open(general_data_ini_file, "r") as reader:
            current_section = ""
            lines = reader.readlines()
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split("=")[0].strip()
                    value = line.split("=")[1].strip()
                    if (current_section, key) in expected_val:
                        assert value == expected_val[(current_section, key)]
                else:
                    current_section = line.strip()

    @staticmethod
    def get_settings_dir(antares_study_path: Path):
        return antares_study_path / "settings"


class TestAntaresDriver:
    def setup_method(self):
        # TODO update antares version which comes with named problems
        # self.nammed_problems = version.parse(__antares_simulator_version__) >= version.parse("8.6")
        self.nammed_problems = True

    def test_antares_cmd(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin1"
        antares_driver = AntaresDriver(exe_path)
        # mock subprocess.run
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, 1)
            expected_cmd = [exe_path, study_dir, "--force-parallel", "1", "-z", "--use-ortools",
                            "--ortools-solver=sirius"]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")

            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

    def test_antares_cmd_force_parallel_option(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin2"
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)

            expected_cmd = [exe_path, study_dir,
                            "--force-parallel", str(n_cpu), "-z", "--use-ortools", "--ortools-solver=sirius"]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

    def test_invalid_n_cpu(self, tmp_path):
        study_dir = tmp_path
        exe_path = "/Path/to/bin"
        n_cpu = -1
        expected_n_cpu = 1
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [
                exe_path,
                study_dir,
                "--force-parallel",
                str(expected_n_cpu),
                "-z", "--use-ortools", "--ortools-solver=sirius"
            ]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

    def test_remove_log_file(self, tmp_path):
        study_dir = tmp_path
        exe_path = tmp_path
        log_file = str(exe_path) + ".log"
        log_file = Path(log_file).touch()
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [str(exe_path), study_dir,
                            "--force-parallel", str(n_cpu), "-z", "--use-ortools", "--ortools-solver=sirius"]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

    def test_non_valid_exe_empty(self, tmp_path):
        study_dir = tmp_path
        settings_dir = study_dir / "settings"
        settings_dir.mkdir()
        antares_driver = AntaresDriver("")
        with pytest.raises(OSError):
            antares_driver.launch(study_dir, 1)

    def test_empty_study_dir(self, tmp_path):

        study_dir = tmp_path
        os.listdir()
        antares_driver = AntaresDriver(get_antares_solver_path())

        with pytest.raises(AntaresDriver.AntaresExecutionError):
            antares_driver.launch(study_dir, 1)

    def initialize_dummy_study_dir(self, study_dir):
        settings_dir = study_dir / "settings"
        settings_dir.mkdir()
        general_data_path = settings_dir / "generaldata.ini"
        default_val = (
            "[general] \n"
            "mode = expansion\n"
            "[optimization] \n"
            "include-exportmps = false\n"
            "include-tc-minstablepower = false\n"
            "include-dayahead = NO\n"
            "include-usexprs = value\n"
            "include-inbasis = value\n"
            "include-outbasis = value\n"
            "include-trace = value\n"
            "[other preferences] \n"
            "unit-commitment-mode = dada\n"
            "[random_section] \n"
            "key1 = value1\n"
            "key2 = value2\n"
        )
        general_data_path.write_text(default_val)

    def test_preserve_adequacy_option_after_run(self, tmp_path):
        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / "generaldata.ini"

        with open(gen_data_path, "w") as writer:
            writer.write("[adequacy patch]\ndummy=false\nfoo = bar\n")
            writer.write("include-adq-patch = true\n")

        study_dir = tmp_path
        exe_path = tmp_path
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [str(exe_path), study_dir,
                            "--force-parallel", str(n_cpu), "-z", "--use-ortools", "--ortools-solver=sirius"]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

        config_reader = configparser.ConfigParser(strict=False)
        config_reader.read(gen_data_path)
        assert config_reader.getboolean("adequacy patch", "dummy") is False
        assert config_reader.get("adequacy patch", "foo") == "bar"
        assert config_reader.getboolean(
            "adequacy patch", "include-adq-patch") is True

    def test_preserve_general_file_section_missing(self, tmp_path):
        settings_dir = TestGeneralDataProcessor.get_settings_dir(tmp_path)
        settings_dir.mkdir()
        gen_data_path = settings_dir / "generaldata.ini"

        study_dir = tmp_path
        exe_path = tmp_path
        n_cpu = 13
        antares_driver = AntaresDriver(exe_path)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            antares_driver.launch(study_dir, n_cpu)
            expected_cmd = [str(exe_path), study_dir,
                            "--force-parallel", str(n_cpu), "-z", "--use-ortools", "--ortools-solver=sirius"]
            if (self.nammed_problems):
                expected_cmd.append("--named-mps-problems")
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=-3, stderr=-3
            )

        config_reader = configparser.ConfigParser(strict=False)
        config_reader.read(gen_data_path)
        assert config_reader.has_section("adequacy patch") is False
