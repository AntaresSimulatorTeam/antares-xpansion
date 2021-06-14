import os
from pathlib import Path

from antares_xpansion.study_output_cleaner import remove_files_containing_str_from_dir, StudyOutputCleaner
from file_creation import _create_empty_file_from_list

def _check_result(tmp_path: Path, removed_files, keep_files):
    keep_files_result = os.listdir(tmp_path)
    for file in removed_files:
        assert (file not in keep_files_result)
    for file in keep_files:
        assert (file in keep_files_result)


class TestStudyOutputCleaner:

    def test_remove_files(self, tmp_path):
        removed_files = ("test", "otherfile_test.txt", "test.txt")
        keep_files = ("otherfile.txt", "any")
        _create_empty_file_from_list(tmp_path, removed_files)
        _create_empty_file_from_list(tmp_path, keep_files)

        remove_files_containing_str_from_dir("test", tmp_path)
        _check_result(tmp_path, removed_files, keep_files)

    def test_antares_step_clean(self, tmp_path):
        removed_files = (
        "problem-year-week-date-hour-1.mps", "criterion-year-week-date-hour-1.txt", "criterion-year-week-date-hour.txt",
        "other-year-week-date-hour-1.txt")
        keep_files = ("problem-year-week-date-hour.mps", "other-year-week-date-hour.txt")
        _create_empty_file_from_list(tmp_path, removed_files)
        _create_empty_file_from_list(tmp_path, keep_files)

        StudyOutputCleaner.clean_antares_step(tmp_path)
        _check_result(tmp_path, removed_files, keep_files)

    def test_lpnamer_step_clean(self, tmp_path):
        removed_files = ("test.mps", "constraints.txt", "constraints-2.txt")
        keep_files = ("mps.txt", "test.txt")
        _create_empty_file_from_list(tmp_path, removed_files)
        _create_empty_file_from_list(tmp_path, keep_files)

        StudyOutputCleaner.clean_lpnamer_step(tmp_path)
        _check_result(tmp_path, removed_files, keep_files)

    def test_benders_step_clean(self, tmp_path):
        os.mkdir(tmp_path / "lp")

        removed_files_lp = ("test.mps", "test.lp", "testother.mps")
        keep_files_lp = ("test.txt", "test.json", "option.txt")
        keep_files_root = ("mps.txt", "area.txt")

        _create_empty_file_from_list(tmp_path / 'lp', removed_files_lp)
        _create_empty_file_from_list(tmp_path / 'lp', keep_files_lp)
        _create_empty_file_from_list(tmp_path, keep_files_root)

        StudyOutputCleaner.clean_benders_step(tmp_path)

        _check_result(tmp_path, (), keep_files_root)
        _check_result(tmp_path / "lp", removed_files_lp, keep_files_lp)

    def test_study_update_step_clean(self, tmp_path):
        remove_files_root = ("interco.txt", "interco", "mps.txt", "area.txt", "area")
        keep_files_root = ("study", "integrity.txt", "othermpsfile.txt")

        _create_empty_file_from_list(tmp_path, remove_files_root)
        _create_empty_file_from_list(tmp_path, keep_files_root)

        StudyOutputCleaner.clean_study_update_step(tmp_path)

        _check_result(tmp_path, remove_files_root, keep_files_root)
