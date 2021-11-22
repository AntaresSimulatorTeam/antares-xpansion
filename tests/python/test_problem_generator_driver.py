from typing import Text
import pytest
import os
from pathlib import Path
from datetime import date, datetime
from unittest.mock import ANY, mock_open, patch

from .file_creation import _create_weight_file
from antares_xpansion.problem_generator_driver import ProblemGeneratorData, ProblemGeneratorDriver
from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.general_data_reader import GeneralDataIniReader

from tests.build_config_reader import get_install_dir, get_lp_namer_exe, get_antares_solver_path

SUBPROCESS_RUN = "antares_xpansion.problem_generator_driver.subprocess.run"


class TestProblemGeneratorDriver:
    number = 0

    def setup_method(self):
        self.lp_exe = "lp_namer.exe"
        self.empty_pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                                        additional_constraints="",
                                                        user_weights_file_path=Path(""),
                                                        weight_file_name_for_lp="",
                                                        lp_namer_exe_path=Path(""),
                                                        nb_active_years=0)

    def test_problem_generator_data(self):

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)

        assert problem_generator_driver.keep_mps == self.empty_pblm_gen_data.keep_mps
        assert problem_generator_driver.additional_constraints == self.empty_pblm_gen_data.additional_constraints
        assert problem_generator_driver.user_weights_file_path == self.empty_pblm_gen_data.user_weights_file_path
        assert problem_generator_driver.lp_namer_exe_path == self.empty_pblm_gen_data.lp_namer_exe_path

    def test_output_path(self, tmp_path):
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.OutputPathError):
            problem_generator_driver.launch(tmp_path / "i_don_t_exist", False)

    def test_no_area_file(self, tmp_path):

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_area_file(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        self._create_empty_area_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)

    def test_no_interco_file(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_interco_file(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)

    def test_lp_namer_exe_not_exit(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

    def test_mps_txt_creation(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        mps_txt_file = tmp_path / "mps.txt"
        assert mps_txt_file.exists()

    def test_mps_txt_content(self, tmp_path):

        expected_results = self._get_expected_mps_txt(tmp_path)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        mps_txt_file = tmp_path / problem_generator_driver.MPS_TXT
        assert mps_txt_file.exists()

        with open(mps_txt_file) as mps_txt:
            for line in mps_txt:
                my_list = line.strip().split(" ")
                assert my_list in expected_results

    def test_lp_namer_log_filename(self, tmp_path):

        lp_exe_file = tmp_path / self.lp_exe
        lp_exe_file.write_text("")
        additional_constraints = "my additionals constraints"
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints=additional_constraints,
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path=lp_exe_file,
                                             nb_active_years=1)

        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        problem_generator_driver.output_path = tmp_path
        assert problem_generator_driver.get_lp_namer_log_filename() == os.path.join(tmp_path, "lp",
                                                                                    self.lp_exe + '.log')

    def test_lp_namer_log_filename_with_non_existing_lp_dir(self):

        additional_constraints = "my additionals constraints"
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints=additional_constraints,
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path="",
                                             nb_active_years=1)

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerPathError):
            problem_generator_driver.get_lp_namer_log_filename()

    def test_clear_old_log(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        log_file_name = self.lp_exe + ".log"
        log_file = tmp_path / log_file_name
        log_file.write_text("bla bla")
        assert log_file.exists()

        pblm_gen = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True):
            with pytest.raises(ProblemGeneratorDriver.LPNamerExecutionError):
                pblm_gen.launch(tmp_path, False)

        assert not log_file.exists()

    def test_clean_lp_dir_before_run(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        log_file_name = self.lp_exe + ".log"
        log_file = tmp_path / log_file_name
        log_file.write_text("bla bla")

        lp_dir = tmp_path / "lp"
        lp_dir.mkdir()
        lp_dir_sub_file_1 = lp_dir / "file1"
        lp_dir_sub_file_1.write_text("")
        lp_dir_sub_file_2 = lp_dir / "file2"
        lp_dir_sub_file_2.write_text("")

        assert lp_dir.exists()
        assert lp_dir_sub_file_1.exists()
        assert lp_dir_sub_file_2.exists()
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            problem_generator_driver.launch(tmp_path, False)

        assert lp_dir.exists()
        assert not lp_dir_sub_file_1.exists()
        assert not lp_dir_sub_file_2.exists()

    def test_weight_file_name_fails_if_file_does_not_exist(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        file_path: Path = tmp_path / "toto_file"
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=file_path,
                                             weight_file_name_for_lp=Path(""),
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        expected_message = f'Illegal value : {str(file_path)} is not an existent yearly-weights file'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(FileNotFoundError) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_weight_file_name_fails_if_there_is_one_negative_value(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [1, -1]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=file_path,
                                             weight_file_name_for_lp=weight_file_name,
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=2)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        expected_message = f'Line 2 in file {str(file_path)} indicates a negative value'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.InvalidYearsWeightValue) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_weight_file_name_fails_if_all_values_are_zero(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [0, 0]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=file_path,
                                             weight_file_name_for_lp=weight_file_name,
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=2)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        expected_message = f'file {str(file_path)} : all values are null'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.OnlyNullYearsWeightValue) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_fails_if_nb_activated_years_different_from_content(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [1, 2, 3, 4]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=file_path,
                                             weight_file_name_for_lp=weight_file_name,
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=5)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        expected_message = f'file {str(file_path)} : invalid weight number : 4 values / 5 expected'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.InvalidYearsWeightNumber) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_weight_file_is_created(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [1, 2]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=file_path,
                                             weight_file_name_for_lp=weight_file_name,
                                             lp_namer_exe_path=lp_namer_file,
                                             nb_active_years=2)

        list_generated_files = self._get_expected_mps_txt(tmp_path)
        mps_files = [week_files[0] for week_files in list_generated_files]
        expected_weight_file_content = []
        for file in mps_files:
            year = self._get_year_index_from_name(file)
            expected_weight_file_content.append(Path(file).with_suffix('').name + " " + str(float(weight_list[year - 1])) + "\n") 
        expected_weight_file_content.append("WEIGHT_SUM " + str(float(sum(weight_list))))
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0

            problem_generator_driver.launch(tmp_path, False)
        
        assert file_path.exists()
        with open(tmp_path / "lp" / weight_file_name , "r") as file:
            lines = file.readlines()
        
        assert lines == expected_weight_file_content
    def _get_expected_mps_txt(self, tmp_path):

        weeks = [1, 2, 3]
        years = [1, 2]
        expected_results = []

        for year in years:
            for week in weeks:
                fname_mps = self._get_file_name("problem", str(year), str(week), "mps")
                mps_file = tmp_path / fname_mps
                mps_file.write_text("")
                fname_vars = self._get_file_name("variables", str(year), str(week), "txt")
                variables_file = tmp_path / fname_vars
                variables_file.write_text("")
                fname_cts = self._get_file_name("constraints", str(year), str(week), "txt")
                constraints_file = tmp_path / fname_cts
                constraints_file.write_text("")

                expected_results.append([fname_mps, fname_vars, fname_cts])

        return expected_results

    def _get_file_name(self, prefix, year, week, extension):

        month = str(date.today().month) if date.today().month > 9 else "0" + str(date.today().month)
        day = str(date.today().day) if date.today().day > 9 else "0" + str(date.today().day)
        today_date = str(date.today().year) + month + day

        hour = str(datetime.now().hour) if datetime.now().hour > 9 else "0" + str(datetime.now().hour)
        minute = str(datetime.now().minute) if datetime.now().minute > 9 else "0" + str(datetime.now().minute)
        second = str(datetime.now().second) if datetime.now().second > 9 else "0" + str(datetime.now().second)

        what_time = hour + minute + second
        file_name = prefix + "-" + year + "-" + week + "-" + today_date + "-" + what_time + "." + extension
        return file_name

    def _create_empty_area_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "area", "txt")

    def _create_empty_interco_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "interco", "txt")

    def _create_empty_n_file(self, tmp_path, prefix, extension):

        TestProblemGeneratorDriver.number += 1
        fname = prefix + str(TestProblemGeneratorDriver.number) + "." + extension
        file = tmp_path / fname
        file.write_text("")

    @staticmethod
    def _get_year_index_from_name(file_name):
        buffer_l = file_name.strip().split("-")
        year = int(buffer_l[1])
        return year
