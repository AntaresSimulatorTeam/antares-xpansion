import pytest
import sys
from pathlib import Path
from unittest.mock import patch

from antares_xpansion.sensitivity_driver import SensitivityDriver

SUBPROCESS_RUN = "antares_xpansion.sensitivity_driver.subprocess.run"


class TestSensitivityDriver:
    def test_non_existing_output_path(self, tmp_path):
        sensitivity_driver = SensitivityDriver("")
        with pytest.raises(SensitivityDriver.SensitivityOutputPathError):
            sensitivity_driver.launch(tmp_path / "i_dont_exist", "test", "other_test")
    
    def test_non_existing_json_input(self, tmp_path):
        simulation_path = tmp_path
        sensitivity_driver = SensitivityDriver("")
        with pytest.raises(SensitivityDriver.SensitivityJsonFilePathError):
            sensitivity_driver.launch(simulation_path, tmp_path / "i_dont_exist", "test")

    def test_sensitivity_cmd(self, tmp_path):
        simulation_path = tmp_path
        exe_path = "/Path/to/bin1"
        sensitivity_driver = SensitivityDriver(exe_path)

        json_in_path = Path(tmp_path / "json_in.json")
        json_in_path.touch()

        json_out_path = "somefile.json"
        
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            sensitivity_driver.launch(simulation_path, json_in_path, json_out_path)
            expected_cmd = [exe_path, "-j", json_in_path, "-o", json_out_path]
            run_function.assert_called_once_with(
                expected_cmd, shell=False, stdout=sys.stdout, stderr=sys.stderr)

    def test_sensitivity_exe_error(self, tmp_path):
        simulation_path = tmp_path
        exe_path = "/Path/to/bin1"
        sensitivity_driver = SensitivityDriver(exe_path)

        json_in_path = Path(tmp_path / "json_in.json")
        json_in_path.touch()

        json_out_path = "somefile.json"
        
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 1
            with pytest.raises(SensitivityDriver.SensitivityExeError):
                sensitivity_driver.launch(simulation_path, json_in_path, json_out_path)
