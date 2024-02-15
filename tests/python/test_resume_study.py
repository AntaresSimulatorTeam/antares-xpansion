import json
from pathlib import Path
import tempfile
from unittest.mock import patch
import pytest
from antares_xpansion.resume_study import ResumeStudy, ResumeStudyData
MOCK_SUBPROCESS_RUN = "antares_xpansion.benders_driver.subprocess.run"


class TestResumeStudy:

    def setup_method(self):
        self.created_ouput_dir = Path(tempfile.mkdtemp()) / "lp"
        self.created_ouput_dir.mkdir()
        self.LAST_MASTER_MPS_KEYWORD = "LAST_MASTER_MPS"
        self.MASTER_MPS_KEYWORD = "MASTER_NAME"
        self.last_master_file = "master_last_iteration"
        self.master_file = "master"
        self.MPS_SUFFIX = ".mps"
        options = {self.LAST_MASTER_MPS_KEYWORD: self.last_master_file,
                   self.MASTER_MPS_KEYWORD: self.master_file}

        self.created_option_file = self.created_ouput_dir / "options.json"
        with open(self.created_option_file, 'w') as options_file:
            json.dump(
                options, options_file, indent=4)

    def test_fail_if_simulation_output_path_not_found(self, tmp_path):

        output_path = tmp_path / "noDir"

        with pytest.raises(ResumeStudy.StudyOutputPathError):
            ResumeStudy(ResumeStudyData(
                output_path, "", "", "", ""))

    def test_fail_if_launcher_options_is_not_found(self, tmp_path):

        output_path = tmp_path / "lp"
        output_path.mkdir()
        with pytest.raises(ResumeStudy.ResumeOptionsFileNotFound):
            resume_study = ResumeStudy(ResumeStudyData(
                output_path, "fefe", "", "", ""))

    def test_fail_if_launcher_options_is_empty(self, tmp_path):
        output_path = tmp_path / "lp"
        output_path.mkdir()
        launcher_options_file = output_path / "launcher_options.json"
        launcher_options_file.touch()
        with pytest.raises(json.decoder.JSONDecodeError):
            ResumeStudy(ResumeStudyData(
                output_path, launcher_options_file.name, "fefe", "", ""))

    def test_fail_if_options_file_not_found(self, tmp_path):
        output_path = tmp_path / "lp"
        output_path.mkdir()
        launcher_options_file = output_path / "launcher_options.json"
        launcher_options_file.write_text("{}")
        with pytest.raises(ResumeStudy.OptionsFileNotFound):
            resume_study = ResumeStudy(ResumeStudyData(
                output_path, launcher_options_file.name, "fefe", "", ""))
            resume_study.launch()

    def test_fail_if_last_master_file_keyword_is_not_found(self, tmp_path):

        output_path = tmp_path / "lp"
        output_path.mkdir()
        launcher_options_file = output_path / "launcher_options.json"
        launcher_options_file.write_text("{}")

        options = {"hello": 31,
                   self.MASTER_MPS_KEYWORD: self.master_file}

        benders_options_file_name = "options.json"
        option_file = output_path / benders_options_file_name
        with open(option_file, 'w') as options_writer:
            json.dump(
                options, options_writer, indent=4)

        with pytest.raises(ResumeStudy.KeyWordNotFound):
            resume_study = ResumeStudy(ResumeStudyData(
                output_path, launcher_options_file.name, benders_options_file_name, "", ""))
            resume_study.launch()

    def test_fail_if_last_master_file_not_found(self, tmp_path):

        output_path = tmp_path / "lp"
        output_path.mkdir()
        launcher_options_file = output_path / "launcher_options.json"
        launcher_options_file.write_text("{}")

        options = {self.LAST_MASTER_MPS_KEYWORD: self.last_master_file,
                   self.MASTER_MPS_KEYWORD: self.master_file}

        benders_options_file_name = "options.json"
        option_file = output_path / benders_options_file_name
        with open(option_file, 'w') as options_writer:
            json.dump(
                options, options_writer, indent=4)

        with pytest.raises(ResumeStudy.LastMasterFileNotFound):
            resume_study = ResumeStudy(ResumeStudyData(
                output_path, launcher_options_file.name, benders_options_file_name,  "", ""))
            resume_study.launch()

    def test_fail_if_options_file_is_updated(self, tmp_path):

        output_path = tmp_path / "lp"
        output_path.mkdir()
        launcher_options_file = output_path / "launcher_options.json"
        launcher_options_file.write_text("{}")

        options = {self.LAST_MASTER_MPS_KEYWORD: self.last_master_file,
                   self.MASTER_MPS_KEYWORD: self.master_file}

        benders_options_file_name = "options.json"
        option_file = output_path / benders_options_file_name
        with open(option_file, 'w') as options_writer:
            json.dump(
                options, options_writer, indent=4)

        create_last_master_file = output_path / \
            (self.last_master_file + self.MPS_SUFFIX)
        create_last_master_file.touch()
        expected_last_master_file_content = "last master file content"
        create_last_master_file.write_text(expected_last_master_file_content)

        create_master_file = output_path / \
            (self.master_file + self.MPS_SUFFIX)
        create_master_file.touch()
        create_master_file.write_text("master file content")

        resume_study = ResumeStudy(ResumeStudyData(
            output_path, launcher_options_file.name, benders_options_file_name,  "", ""))
        with patch(MOCK_SUBPROCESS_RUN, autospec=True) as run_function:
            run_function.return_value.returncode = 0
            resume_study.launch()
