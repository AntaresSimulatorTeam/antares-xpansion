import json
from pathlib import Path
import tempfile
import pytest
from antares_xpansion.resume_study import ResumeStudy


class TestResumeStudy:

    def setup_method(self):
        self.created_ouput_dir = Path(tempfile.mkdtemp()) / "lp"
        self.created_ouput_dir.mkdir()
        self.LAST_MASTER_MPS_KEYWORD = "LAST_MASTER_MPS"
        self.MASTER_MPS_KEYWORD = "MASTER"
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

        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.StudyOutputPathError):
            resume_study.launch(output_path, "hello", "hello", "hello")

    def test_fail_if_options_file_not_found(self, tmp_path):
        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.OptionsFileNotFound):
            resume_study.launch(self.created_ouput_dir,
                                "hello", "hello", "master")

    def test_fail_if_last_master_file_invalid_keyword(self, tmp_path):

        resume_study = ResumeStudy()

        with pytest.raises(ResumeStudy.KeyWordNotFound):
            resume_study.launch(self.created_ouput_dir,
                                self.created_option_file.name, "hello", "master")

    def test_fail_if_last_master_file_not_found(self, tmp_path):

        resume_study = ResumeStudy()

        with pytest.raises(ResumeStudy.LastMasterFileNotFound):
            resume_study.launch(self.created_ouput_dir,
                                self.created_option_file.name, self.LAST_MASTER_MPS_KEYWORD, self.MASTER_MPS_KEYWORD)

    def test_fail_if_options_file_is_updated(self, tmp_path):

        resume_study = ResumeStudy()
        create_last_master_file = self.created_ouput_dir / \
            (self.last_master_file + self.MPS_SUFFIX)
        create_last_master_file.touch()
        expected_last_master_file_content = "last master file content"
        create_last_master_file.write_text(expected_last_master_file_content)

        create_master_file = self.created_ouput_dir / \
            (self.master_file + self.MPS_SUFFIX)
        create_master_file.touch()
        create_master_file.write_text("master file content")

        resume_study.launch(self.created_ouput_dir,
                            self.created_option_file.name, self.LAST_MASTER_MPS_KEYWORD, self.MASTER_MPS_KEYWORD)

        with open(self.created_option_file, "r") as options_json:
            options = json.load(options_json)

        assert options[self.MASTER_MPS_KEYWORD] == options[self.LAST_MASTER_MPS_KEYWORD]

    def test_fail_if_master_file_is_not_updated(self, tmp_path):

        resume_study = ResumeStudy()
        create_last_master_file = self.created_ouput_dir / \
            (self.last_master_file + self.MPS_SUFFIX)
        create_last_master_file.touch()
        expected_last_master_file_content = "last master file content"
        create_last_master_file.write_text(expected_last_master_file_content)

        create_master_file = self.created_ouput_dir / \
            (self.master_file + self.MPS_SUFFIX)
        create_master_file.touch()
        create_master_file.write_text("master file content")

        resume_study.launch(self.created_ouput_dir,
                            self.created_option_file.name, self.LAST_MASTER_MPS_KEYWORD, self.MASTER_MPS_KEYWORD)

        with open(self.created_option_file, "r") as options_json:
            options = json.load(options_json)

        assert options[self.MASTER_MPS_KEYWORD] == options[self.LAST_MASTER_MPS_KEYWORD]

        with open(create_last_master_file) as last_master_file, open(create_master_file) as master_file:
            last_master_file_content = last_master_file.readlines()
            master_file_content = master_file.readlines()
            assert master_file_content == [expected_last_master_file_content]
            assert master_file_content == last_master_file_content
