"""
    Class to control the execution of the optimization session
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.benders_driver import BendersDriver, SolversExe
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.full_run_driver import FullRunDriver
from antares_xpansion.general_data_processor import GeneralDataProcessor
from antares_xpansion.logger import step_logger
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver, ProblemGeneratorData
from antares_xpansion.resume_study import ResumeStudy, ResumeStudyData
from antares_xpansion.sensitivity_driver import SensitivityDriver
from antares_xpansion.study_updater_driver import StudyUpdaterDriver


class XpansionDriver:
    """
    Class to control the execution of the optimization session
    """

    def __init__(self, config_loader: ConfigLoader):
        """
        Initialise driver with a given antaresXpansion configuration,
        the system platform and parses the arguments
        :param config: configuration to use for the optimization
        :type config: XpansionConfig object
        """

        self.config_loader = config_loader
        self.logger = step_logger(__name__, __class__.__name__)
        self.antares_driver = AntaresDriver(
            self.config_loader.antares_exe()
        )
        data = ProblemGeneratorData(keep_mps=self.config_loader.keep_mps(),
                                    additional_constraints=self.config_loader.additional_constraints(),
                                    user_weights_file_path=self.config_loader.weights_file_path(),
                                    weight_file_name_for_lp=self.config_loader.weight_file_name(),
                                    lp_namer_exe_path=self.config_loader.lp_namer_exe(),
                                    active_years=self.config_loader.active_years,
                                    memory=self.config_loader.memory(),
                                    )
        self.problem_generator_driver = ProblemGeneratorDriver(data)

        self.benders_driver = BendersDriver(
            SolversExe(
                self.config_loader.benders_exe(),
                self.config_loader.merge_mps_exe(),
                self.config_loader.outer_loop_exe()),
            self.config_loader.options_file_name(),
            self.config_loader.mpi_exe(),
        )

        self.study_update_driver = StudyUpdaterDriver(
            self.config_loader.study_update_exe())

        self.sensitivity_driver = SensitivityDriver(
            self.config_loader.sensitivity_exe())

        self.full_run_driver = FullRunDriver(self.config_loader.full_run_exe(
        ), self.problem_generator_driver, self.benders_driver)
        self.settings = 'settings'

    def launch(self):
        """
        launch antares xpansion steps
        """

        if self.config_loader.step() == "full" and not self.config_loader.memory():
            self.launch_antares_step()
            self.logger.info("Post Antares")
            self.problem_generator_driver.set_output_path(
                self.config_loader.simulation_output_path())
            self.problem_generator_driver.create_lp_dir()
            self.config_loader.benders_pre_actions()
            self.full_run_driver.launch(self.config_loader.simulation_output_path(),
                                        self.config_loader.is_relaxed(),
                                        self.config_loader.method(),
                                        self.config_loader.json_file_path(),
                                        self.config_loader.keep_mps(),
                                        self.config_loader.n_mpi(),
                                        self.config_loader.oversubscribe(),
                                        self.config_loader.allow_run_as_root())
            self.clean_step()

        elif self.config_loader.step() == "full" and self.config_loader.memory():
            self.update_study_settings(memory_mode=True)
            self.launch_problem_generation_step_memory()
            self.launch_benders_step()
            self.study_update_driver.launch(
                self.config_loader.xpansion_simulation_output(), self.config_loader.json_file_path(),
                self.config_loader.keep_mps())

        elif self.config_loader.step() == "antares":
            self.launch_antares_step()

        elif self.config_loader.step() == "problem_generation":
            self.launch_problem_generation_step()

        elif self.config_loader.step() == "study_update":
            self.study_update_driver.launch(
                self.config_loader.xpansion_simulation_output(), self.config_loader.json_file_path(), self.config_loader.keep_mps())

        elif self.config_loader.step() == "benders":
            self.launch_benders_step()

        elif self.config_loader.step() == "sensitivity":
            self.launch_sensitivity_step()

        elif self.config_loader.step() == "resume":
            self.resume_study()

        else:
            raise XpansionDriver.UnknownStep(
                f"Launching failed! {self.config_loader.step()} is not an Xpansion step.")

    def clean_step(self):
        ret = subprocess.run(
            [str(self.config_loader.antares_archive_updater_exe()), "-a", str(self.config_loader.simulation_output_path()),
             "-p", self.config_loader.simulation_lp_path(), self.config_loader.expansion_dir(), "-d"], shell=False, stdout=sys.stdout, stderr=sys.stderr,
            encoding='utf-8')
        if ret.returncode != 0:
            raise XpansionDriver.AntaresArchiveUpdaterExeError(
                f"ERROR: exited {self.config_loader.antares_archive_updater_exe()} with status {ret.returncode}"
            )
        shutil.rmtree(self.config_loader.xpansion_simulation_output())

    def launch_antares_step(self):
        self.update_study_settings(memory_mode=False)

        use_xpress = str(self.config_loader.options["solver"]).upper() == "XPRESS"

        try:
            ret = self.antares_driver.launch(
                self.config_loader.data_dir(), self.config_loader.antares_n_cpu(), use_xpress)
            if ret is False:
                self._revert_general_data_ini()
            else:
                self._backup_general_data_ini_on_error()
                self._revert_general_data_ini()
        except AntaresDriver.AntaresExecutionError as e:
            self.logger.error(
                "Antares exited with error, backup current general data file and revert original one")
            self._backup_general_data_ini_on_error()
            self._revert_general_data_ini()
            raise e

    def update_study_settings(self, memory_mode=False):
        self._configure_general_data_processor()
        self._backup_general_data_ini()
        self._update_general_data_ini(memory_mode)

    def _update_general_data_ini(self, memory_mode=False):
        self.gen_data_proc.change_general_data_file_to_configure_antares_execution(memory_mode)

    def launch_problem_generation_step(self):
        self.problem_generator_driver.launch(
            self.config_loader.simulation_output_path(), self.config_loader.is_relaxed())

    def launch_problem_generation_step_memory(self):
        self.problem_generator_driver.launch_memory(self.config_loader.data_dir(), self.config_loader.is_relaxed())

    def launch_benders_step(self):
        self.config_loader.benders_pre_actions()
        self.benders_driver.launch(
            self.config_loader.xpansion_simulation_output(),
            self.config_loader.method(),
            self.config_loader.keep_mps(),
            self.config_loader.n_mpi(),
            oversubscribe=self.config_loader.oversubscribe(),
            allow_run_as_root=self.config_loader.allow_run_as_root()
        )

    def launch_sensitivity_step(self):
        self.config_loader._create_sensitivity_dir()

        self.sensitivity_driver.launch(
            self.config_loader.simulation_output_path(),
            self.config_loader.json_sensitivity_in_path(),
            self.config_loader.json_file_path(),
            self.config_loader.last_master_file_path(),
            self.config_loader.last_master_basis_path(),
            self.config_loader.structure_file_path(),
            self.config_loader.json_sensitivity_out_path(),
            self.config_loader.sensitivity_log_file(),
            self.config_loader.xpansion_simulation_output()
        )
        self.config_loader.update_last_study_with_sensitivity_results()

    def resume_study(self):
        self.config_loader.benders_pre_actions()
        resume_study_data = ResumeStudyData(
            Path(self.config_loader.simulation_lp_path()),
            self.config_loader.launcher_options_file_path(),
            self.config_loader.options_file_name(),
            self.config_loader.benders_exe(),
            self.config_loader.merge_mps_exe())

        resume_study = ResumeStudy(resume_study_data)
        resume_study.launch()

    class UnknownStep(Exception):
        pass

    class AntaresArchiveUpdaterExeError(Exception):
        pass

    def _revert_general_data_ini(self):
        self.gen_data_proc.revert_backup_data()

    def _backup_general_data_ini(self):
        self.gen_data_proc.backup_data()

    def _configure_general_data_processor(self):
        settings_dir = os.path.normpath(
            os.path.join(self.config_loader.data_dir(), self.settings)
        )
        self.gen_data_proc = GeneralDataProcessor(
            settings_dir, self.config_loader.is_accurate()
        )

    def _backup_general_data_ini_on_error(self):
        self.gen_data_proc.backup_data_on_error()
