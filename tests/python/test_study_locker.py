from pathlib import Path
import pytest

from antares_xpansion.study_locker import StudyLocker
class TestStudyLocker:


    def test_non_existing_study_dir(self, tmp_path):

        with pytest.raises(StudyLocker.NotAValidDirectory):
            StudyLocker(tmp_path / "ghost")
    

    def test_fail_to_unlocked_already_locked_dir(self, tmp_path):

        study_dir = tmp_path / "secret_study"
        _ = TestStudyLocker._get_a_locked_study(study_dir)

        with pytest.raises(StudyLocker.Locked):
            _ = TestStudyLocker._get_a_locked_study(study_dir)
    
    
    def test_locker_file_contains_multiple_lines(self, tmp_path):
        study_dir = tmp_path / "secret_study"
        study_locker_1 = TestStudyLocker._get_a_locked_study(study_dir)

        assert study_locker_1.locker_file.exists()
        with open(study_locker_1.locker_file, "w") as file :
            lines = ["line 1 \n", "\n line 2 \n"]
            file.writelines(lines)
        
        with pytest.raises(StudyLocker.CorruptedLockerFile):
            _ = TestStudyLocker._get_a_locked_study(study_dir)
    

    def test_locker_file_corrupted(self, tmp_path):
        study_dir = tmp_path / "secret_study"
        study_locker_1 = TestStudyLocker._get_a_locked_study(study_dir)

        assert study_locker_1.locker_file.exists()
        with open(study_locker_1.locker_file, "w") as file :
            line = ["something = "+ str(123)]
            file.writelines(line)
        
        with pytest.raises(StudyLocker.CorruptedLockerFile):
            _ = TestStudyLocker._get_a_locked_study(study_dir)
    

    def test_locker_file_corrupted_pid_is_not_an_int(self, tmp_path):
        study_dir = tmp_path / "secret_study"
        study_locker_1 = TestStudyLocker._get_a_locked_study(study_dir)

        assert study_locker_1.locker_file.exists()
        with open(study_locker_1.locker_file, "w") as file :
            line = ["PID = not_an_int"]
            file.writelines(line)
        
        with pytest.raises(StudyLocker.CorruptedLockerFile):
            _ = TestStudyLocker._get_a_locked_study(study_dir)
    

    def test_empty_locker_file(self, tmp_path):
        study_dir = tmp_path / "secret_study"
        study_dir.mkdir()

        locker_file = study_dir / ".xpansion_locker"
        locker_file.write_text("")

        _ = TestStudyLocker._get_a_locked_study(study_dir)


    @staticmethod
    def _get_a_locked_study(study_dir : Path):

        if not study_dir.is_dir():
            study_dir.mkdir()

        study_locker = StudyLocker(study_dir)
        study_locker.lock()
        return study_locker