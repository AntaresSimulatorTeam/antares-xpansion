import os
import shutil
import tempfile
from pathlib import Path
import pytest

from antares_xpansion.study_locker import StudyLocker


class TestStudyLocker:
    def setup_method(self):
        self.study_dir = Path(tempfile.mkdtemp())
        self.lock_file = self.study_dir / ".xpansion_locker"

    def teardown_method(self):
        shutil.rmtree(self.study_dir)

    def test_non_existing_study_dir(self, tmp_path):
        with pytest.raises(StudyLocker.NotAValidDirectory):
            StudyLocker(tmp_path / "ghost")

    def test_lock_study_create_lock_file(self):
        TestStudyLocker.lock_study(self.study_dir)
        assert self.lock_file.exists()
        assert os.stat(self.lock_file).st_size > 0

    def test_study_with_empty_locker_file_can_be_locked(self, tmp_path):
        self.lock_file.write_text("")
        TestStudyLocker.lock_study(self.study_dir)

    def test_fail_to_unlock_already_locked_dir(self):
        TestStudyLocker.lock_study(self.study_dir)
        with pytest.raises(StudyLocker.Locked):
            TestStudyLocker.lock_study(self.study_dir)

    def test_locker_file_contains_multiple_lines_raises_exception(self):
        with open(self.lock_file, "w") as file:
            lines = ["line 1 \n", "\n line 2 \n"]
            file.writelines(lines)
        with pytest.raises(StudyLocker.CorruptedLockerFile):
            TestStudyLocker.lock_study(self.study_dir)

    def test_locker_file_with_one_non_valid_line_raises_exception(self):
        with open(self.lock_file, "w") as file:
            line = ["something = " + str(123)]
            file.writelines(line)

        with pytest.raises(StudyLocker.CorruptedLockerFile):
            TestStudyLocker.lock_study(self.study_dir)

    def test_locker_file_corrupted_pid_is_not_an_int_raises_exception(self):
        with open(self.lock_file, "w") as file:
            line = ["PID = not_an_int"]
            file.writelines(line)

        with pytest.raises(StudyLocker.CorruptedLockerFile):
            TestStudyLocker.lock_study(self.study_dir)

    @staticmethod
    def lock_study(study_dir: Path):
        study_locker = StudyLocker(study_dir)
        study_locker.lock()
