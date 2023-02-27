import os.path
import shutil
import sys
from pathlib import Path
from unittest.mock import patch

import pytest
from antares_xpansion.sensitivity_driver import SensitivityDriver

from file_creation import _create_empty_file_from_list

SUBPROCESS_RUN = "antares_xpansion.sensitivity_driver.subprocess.run"


class TestSensitivityDriver:
    def test_non_existing_output_path(self, tmp_path):
        sensitivity_driver = SensitivityDriver("")
        with pytest.raises(SensitivityDriver.SensitivityOutputPathError):
            sensitivity_driver.launch(
                tmp_path / "i_dont_exist",
                "test",
                "other_test",
                "mock",
                "mock basis",
                "mock",
                "mock",
                "a_path",
                tmp_path,
            )

    def test_non_existing_input_file(self, tmp_path):
        simulation_path = tmp_path
        sensitivity_driver = SensitivityDriver("")
        with pytest.raises(SensitivityDriver.SensitivityOutputPathError):
            sensitivity_driver.launch(
                simulation_path,
                tmp_path / "i_dont_exist",
                "test",
                "",
                "",
                "",
                "",
                "a_path",
                tmp_path,
            )

    def test_sensitivity_cmd(self, tmp_path):
        simulation_path = os.path.join(tmp_path, "output")
        Path(simulation_path).mkdir()
        exe_path = "/Path/to/bin1"
        sensitivity_driver = SensitivityDriver(exe_path)

        input_files = ["json_in.json", "benders_out", "master", "my_basis", "structure"]
        input_paths = list(map(lambda x: os.path.join(simulation_path, x), input_files))
        _create_empty_file_from_list(simulation_path, input_files)
        shutil.make_archive(simulation_path, "zip", simulation_path)

        json_out_path = "somefile.json"
        sensitivity_log_path = "a_path"

        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            sensitivity_driver.launch(
                f"{simulation_path}.zip",
                input_paths[0],
                input_paths[1],
                input_paths[2],
                input_paths[3],
                input_paths[4],
                json_out_path,
                sensitivity_log_path,
                tmp_path,
            )
            expected_cmd = [
                exe_path,
                "-i",
                input_paths[0],
                "-b",
                input_paths[1],
                "-m",
                input_paths[2],
                "-s",
                input_paths[4],
                "-o",
                json_out_path,
                "-l",
                sensitivity_log_path,
                "--basis",
                input_paths[3],
            ]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=sys.stdout, stderr=sys.stderr
            )

    def test_sensitivity_exe_error(self, tmp_path):
        simulation_path = os.path.join(tmp_path, "output")
        Path(simulation_path).mkdir()
        exe_path = "/Path/to/bin1"
        sensitivity_driver = SensitivityDriver(exe_path)

        input_files = ["json_in.json", "benders_out", "master", "my_basis", "structure"]
        input_paths = list(map(lambda x: os.path.join(simulation_path, x), input_files))
        _create_empty_file_from_list(simulation_path, input_files)
        shutil.make_archive(simulation_path, "zip", simulation_path)

        json_out_path = "somefile.json"
        sensitivity_log_path = "a_path"

        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 1
            with pytest.raises(SensitivityDriver.SensitivityExeError):
                sensitivity_driver.launch(
                    f"{simulation_path}.zip",
                    input_paths[0],
                    input_paths[1],
                    input_paths[2],
                    input_paths[3],
                    input_paths[4],
                    json_out_path,
                    sensitivity_log_path,
                    tmp_path,
                )
