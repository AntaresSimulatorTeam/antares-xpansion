import shutil
from datetime import date, datetime
from pathlib import Path
from unittest.mock import patch

import pytest
from antares_xpansion.problem_generator_driver import ProblemGeneratorData, ProblemGeneratorDriver

SUBPROCESS_RUN = "antares_xpansion.problem_generator_driver.subprocess.run"
zipfile_ZipFile = "antares_xpansion.problem_generator_driver.zipfile.ZipFile"


def get_zipped_output(tmp_path) -> Path:
    zipped_output = tmp_path.parent / (tmp_path.name+".zip")
    shutil.make_archive(tmp_path, "zip", tmp_path)
    return zipped_output


class TestProblemGeneratorDriver:
    number = 0

    def setup_method(self):
        self.lp_exe = "lp_namer.exe"
        self.empty_pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                                        additional_constraints="",
                                                        user_weights_file_path=Path(
                                                            ""),
                                                        weight_file_name_for_lp="",
                                                        lp_namer_exe_path=Path(
                                                            ""),
                                                        active_years=[],
                                                        memory=False)

    def test_problem_generator_data(self):

        problem_generator_driver = ProblemGeneratorDriver(
            self.empty_pblm_gen_data)

        assert problem_generator_driver.keep_mps == self.empty_pblm_gen_data.keep_mps
        assert problem_generator_driver.additional_constraints == self.empty_pblm_gen_data.additional_constraints
        assert problem_generator_driver.user_weights_file_path == self.empty_pblm_gen_data.user_weights_file_path
        assert problem_generator_driver.lp_namer_exe_path == self.empty_pblm_gen_data.lp_namer_exe_path

    def test_output_path(self, tmp_path):
        problem_generator_driver = ProblemGeneratorDriver(
            self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.OutputPathError):
            problem_generator_driver.launch(tmp_path / "i_don_t_exist", False)

    def test_lp_namer_exe_does_not_exit(self, tmp_path):

        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        zipped_output = get_zipped_output(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(
            self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(zipped_output, False)

    def xpansion_output(self, tmp_path):
        return tmp_path.parent / (tmp_path.name + '-Xpansion')

    def test_clear_old_log(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path=lp_namer_file,
                                             active_years=[],
                                             memory=False)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        output_zipped = get_zipped_output(tmp_path)
        log_file_name = self.lp_exe + ".log"
        xpansion_dir = self.xpansion_output(tmp_path)
        log_file = xpansion_dir / log_file_name
        xpansion_dir.mkdir()
        log_file.write_text("bla bla")
        assert log_file.exists()

        pblm_gen = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True):
            with pytest.raises(ProblemGeneratorDriver.LPNamerExecutionError):
                pblm_gen.launch(output_zipped, False)

        assert not log_file.exists()

    def test_clean_lp_dir_before_run(self, tmp_path):

        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(keep_mps=False,
                                             additional_constraints="",
                                             user_weights_file_path=Path(""),
                                             weight_file_name_for_lp="",
                                             lp_namer_exe_path=lp_namer_file,
                                             active_years=[],
                                             memory=False)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        output_zipped = get_zipped_output(tmp_path)
        log_file_name = self.lp_exe + ".log"
        xpansion_dir = self.xpansion_output(tmp_path)
        xpansion_dir.mkdir()
        log_file = xpansion_dir / log_file_name
        log_file.write_text("bla bla")

        lp_dir = xpansion_dir / "lp"
        lp_dir.mkdir()
        lp_dir_sub_file_1 = lp_dir / "file1"
        lp_dir_sub_file_1.write_text("")

        assert lp_dir.exists()
        assert lp_dir_sub_file_1.exists()
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            problem_generator_driver.launch(output_zipped, False)

        assert lp_dir.exists()
        assert not lp_dir_sub_file_1.exists()

    def _get_expected_mps_txt(self, tmp_path):

        weeks = [1, 2, 3]
        years = [1, 2]
        expected_results = []

        for year in years:
            for week in weeks:
                fname_mps = self._get_file_name(
                    "problem", str(year), str(week), "mps")
                mps_file = tmp_path / fname_mps
                mps_file.write_text("")
                fname_vars = self._get_file_name(
                    "variables", str(year), str(week), "txt")
                variables_file = tmp_path / fname_vars
                variables_file.write_text("")
                fname_cts = self._get_file_name(
                    "constraints", str(year), str(week), "txt")
                constraints_file = tmp_path / fname_cts
                constraints_file.write_text("")

                expected_results.append([fname_mps, fname_vars, fname_cts])

        return expected_results

    def _get_file_name(self, prefix, year, week, extension):

        month = str(date.today().month) if date.today(
        ).month > 9 else "0" + str(date.today().month)
        day = str(date.today().day) if date.today(
        ).day > 9 else "0" + str(date.today().day)
        today_date = str(date.today().year) + month + day

        hour = str(datetime.now().hour) if datetime.now(
        ).hour > 9 else "0" + str(datetime.now().hour)
        minute = str(datetime.now().minute) if datetime.now(
        ).minute > 9 else "0" + str(datetime.now().minute)
        second = str(datetime.now().second) if datetime.now(
        ).second > 9 else "0" + str(datetime.now().second)

        what_time = hour + minute + second
        file_name = prefix + "-" + year + "-" + week + "-" + \
            today_date + "-" + what_time + "." + extension
        return file_name

    def _create_empty_area_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "area", "txt")

    def _create_empty_interco_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "interco", "txt")

    def _create_empty_n_file(self, tmp_path, prefix, extension):

        TestProblemGeneratorDriver.number += 1
        fname = prefix + \
            str(TestProblemGeneratorDriver.number) + "." + extension
        file = tmp_path / fname
        file.write_text("")

    @staticmethod
    def _get_year_index_from_name(file_name):
        buffer_l = file_name.strip().split("-")
        year = int(buffer_l[1])
        return year
