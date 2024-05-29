

from dataclasses import dataclass
import json
from pathlib import Path
import shutil
from antares_xpansion.benders_driver import BendersDriver, SolversExe
from antares_xpansion.launcher_options_default_value import LauncherOptionsDefaultValues
from antares_xpansion.launcher_options_keys import LauncherOptionsKeys
from antares_xpansion.optimisation_keys import OptimisationKeys


@dataclass
class ResumeStudyData:
    simulation_output_path: Path
    launcher_options_file: Path
    benders_options_file: str
    benders_exe: str
    merge_mps_exe: str


class ResumeStudy:
    def __init__(self, resume_study_data: ResumeStudyData) -> None:
        self.MPS_SUFFIX = ".mps"
        self.set_simulation_output_path(
            resume_study_data.simulation_output_path)
        self.launcher_options_file = self._simulation_output_path / \
            resume_study_data.launcher_options_file
        self._load_resume_options()
        self.benders_exe = resume_study_data.benders_exe
        self.merge_mps_exe = resume_study_data.merge_mps_exe
        self.benders_options_file = resume_study_data.benders_options_file

    def _load_resume_options(self):
        if not self.launcher_options_file.exists():
            raise ResumeStudy.ResumeOptionsFileNotFound(
                f"Resume options file {self.launcher_options_file} not found!")

        with open(self.launcher_options_file, "r") as options:
            resume_options = json.load(options)

        self.method = resume_options.get(
            LauncherOptionsKeys.method_key(), LauncherOptionsDefaultValues.DEFAULT_METHOD())
        self.allow_run_as_root = resume_options.get(
            LauncherOptionsKeys.allow_run_as_root_key(), LauncherOptionsDefaultValues.DEFAULT_ALLOW_RUN_AS_ROOT())
        self.oversubscribe = resume_options.get(
            LauncherOptionsKeys.oversubscribe_key(), LauncherOptionsDefaultValues.DEFAULT_OVERSUBSCRIBE())
        self.keep_mps = resume_options.get(LauncherOptionsKeys.keep_mps_key(
        ), LauncherOptionsDefaultValues.DEFAULT_KEEPMPS())
        self.antares_n_cpu = resume_options.get(
            LauncherOptionsKeys.antares_n_cpu_key(), LauncherOptionsDefaultValues.DEFAULT_ANTARES_N_CPU())
        self.n_mpi = resume_options.get(
            LauncherOptionsKeys.n_mpi_key(), LauncherOptionsDefaultValues.DEFAULT_NP())

    def launch(self):

        self._update_options_file()

    def set_simulation_output_path(self, simulation_output_path: Path):
        if simulation_output_path.is_dir():
            self._simulation_output_path = simulation_output_path
        else:
            raise ResumeStudy.StudyOutputPathError(
                f"Study output path error: {simulation_output_path} not found!"
            )

    class StudyOutputPathError(Exception):
        pass

    def _update_options_file(self):

        options_file_path = self._simulation_output_path / self.benders_options_file
        last_master_keyword = OptimisationKeys.last_mps_master_name_key()
        master_keyword = OptimisationKeys.master_name_key()
        if not options_file_path.exists():
            raise ResumeStudy.OptionsFileNotFound(
                f"Options file: {options_file_path} not found")

        options_file_path = self._simulation_output_path / self.benders_options_file
        with open(options_file_path, "r") as json_file:
            options = json.load(json_file)
        try:
            last_master_file = options[last_master_keyword]
        except KeyError:
            raise ResumeStudy.KeyWordNotFound(
                f"Error {last_master_keyword} not found in {options_file_path}")
        last_master_file_path = self._simulation_output_path / \
            (last_master_file + self.MPS_SUFFIX)

        if not last_master_file_path.exists():
            raise ResumeStudy.LastMasterFileNotFound(
                f"last master file: {last_master_file_path} not found")

        try:
            master_file = options[master_keyword]
        except KeyError:
            raise ResumeStudy.KeyWordNotFound(
                f"Error {master_keyword} not found in {options_file_path}")
        master_file_path = self._simulation_output_path / \
            (master_file + self.MPS_SUFFIX)

        shutil.copy(last_master_file_path, master_file_path)

        assert master_file_path.exists()

        options[OptimisationKeys.resume_key()] = True
        with open(options_file_path, "w") as options_json:
            json.dump(options, options_json, indent=4)

        benders_driver = BendersDriver(
            SolversExe(self.benders_exe, self.merge_mps_exe, ""),
            self.benders_options_file
        )
        benders_driver.launch(self._simulation_output_path.parent, self.method,
                              self.keep_mps, self.n_mpi, self.oversubscribe, self.allow_run_as_root)

    class OptionsFileNotFound(Exception):
        pass

    class KeyWordNotFound(Exception):
        pass

    class LastMasterFileNotFound(Exception):
        pass

    class MasterFileNotFound(Exception):
        pass

    class ResumeOptionsFileNotFound(Exception):
        pass
