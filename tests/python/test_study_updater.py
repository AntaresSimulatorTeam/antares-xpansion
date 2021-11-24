import os
from pathlib import Path
from unittest import mock

import pytest

from antares_xpansion.study_updater_driver import StudyUpdaterDriver


class TestStudyUpdater:

    def test_if_study_updater_exe_is_no_valid_file_exception_is_raised(self):
        with pytest.raises(StudyUpdaterDriver.StudyUpdaterOutputPathError):
            study_updater = StudyUpdaterDriver(
                study_updater_exe="toto", json_name="titi")

    def test_if_process_return_code_is_one_exception_is_raised(self,  tmp_path):
        study_updater_exe = tmp_path / "dummy"
        open(study_updater_exe, "a").close()
        study_updater = StudyUpdaterDriver(
            study_updater_exe=str(study_updater_exe), json_name="titi")
        simulation_output_path = tmp_path / "simulation"
        os.mkdir(simulation_output_path)

        with mock.patch("antares_xpansion.study_updater_driver.subprocess.run", autospec=True) as run_function:
            run_function.return_value.returncode = 1
            with pytest.raises(StudyUpdaterDriver.UpdaterExecutionError) as e:
                study_updater.launch(
                    simulation_output_path=simulation_output_path, keep_mps=True)
        assert str(e.value) == f"ERROR: exited study-updater with status 1"

    def test_updater_is_called_with_proper_argument_logfile_is_overwritten(self, tmp_path):
        study_updater_exe = tmp_path / "dummy"
        open(study_updater_exe, "a").close()
        simulation_output_path = tmp_path / "simulation"
        os.mkdir(simulation_output_path)
        logfile = simulation_output_path / (study_updater_exe.name+".log")
        with open(logfile, "a") as f:
            f.write("hello")
        assert os.stat(logfile).st_size == 5

        study_updater = StudyUpdaterDriver(
            study_updater_exe=str(study_updater_exe), json_name="titi")

        with mock.patch("antares_xpansion.study_updater_driver.subprocess.run", autospec=True) as run_function:
            run_function.return_value.returncode = 0
            study_updater.launch(
                simulation_output_path=simulation_output_path, keep_mps=True)

            expected_cmd = [str(study_updater_exe), "-o",
                            str(simulation_output_path), "-s", "titi.json"]
            args, _ = run_function.call_args_list[0]

        assert args[0] == expected_cmd
        assert os.stat(logfile).st_size == 0

    def test_todo_test_call_of_study_cleaner(self):
        pass

    def test_fail_if_simulation_output_non_exist(self, tmp_path):
        study_updater_exe = tmp_path / "dummy"
        open(study_updater_exe, "a").close()
        study_updater = StudyUpdaterDriver(
            study_updater_exe=str(study_updater_exe), json_name="titi")
        with pytest.raises(StudyUpdaterDriver.StudyUpdaterOutputPathError):
            study_updater.launch(
                simulation_output_path=tmp_path / "what", keep_mps=True)
