import pytest
from antares_xpansion.resume_study import ResumeStudy


class TestResumeStudy:

    def test_fail_if_simulation_output_path_not_found(self, tmp_path):
        output_path = tmp_path / "noDir"

        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.StudyOutputPathError):
            resume_study.launch(output_path, "hello")

    def test_fail_if_last_master_file_not_found(self, tmp_path):
        output_path = tmp_path / "lp"
        output_path.mkdir()

        resume_study = ResumeStudy()
        with pytest.raises(ResumeStudy.LastMasterFileNotFound):
            resume_study.launch(output_path, "hello")
