import json
from pathlib import Path
import tempfile
import pytest
from antares_xpansion.resume_study import ResumeStudy


class TestResumeStudy:

    def setup_method(self):
        self.created_ouput_dir = Path(tempfile.mkdtemp()) / "lp"
        self.created_ouput_dir.mkdir()
        self.LAST_MASTER_MPS = "LAST_MASTER_MPS"
        options = {self.LAST_MASTER_MPS: "master_last_iteration"}

        self.created_option_file = self.created_ouput_dir / "options.json"
        with open(self.created_option_file, 'w') as options_file:
            json.dump(
                options, options_file, indent=4)

    def test_fail_if_simulation_output_path_not_found(self, tmp_path):
        output_path = tmp_path / "noDir"

        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.StudyOutputPathError):
            resume_study.launch(output_path, "hello", "hello")

    def test_fail_if_options_file_not_found(self, tmp_path):
        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.OptionsFileNotFound):
            resume_study.launch(self.created_ouput_dir, "hello", "hello")

    def test_fail_if_last_master_file_invalid_keyword(self, tmp_path):

        resume_study = ResumeStudy()

        with pytest.raises(ResumeStudy.LastMasterKeyWordNotFound):
            resume_study.launch(self.created_ouput_dir,
                                self.created_option_file.name, "hello")

    def test_fail_if_last_master_file_not_found(self, tmp_path):

        resume_study = ResumeStudy()

        with pytest.raises(ResumeStudy.LastMasterFileNotFound):
            resume_study.launch(self.created_ouput_dir,
                                self.created_option_file.name, self.LAST_MASTER_MPS)

        # with open(self.created_ouput_dir /
        #           self.created_option_file.name, "r") as json_file:
        #     options = json.load(json_file)

        # assert options
