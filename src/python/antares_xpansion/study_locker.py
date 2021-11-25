import os
from functools import partial
from operator import is_not
from pathlib import Path
import psutil


class StudyLocker:
    """
        class responsible for locking a study
    """

    def __init__(self, study_dir: Path) -> None:
        self.study_dir = Path("")
        self._set_study_dir(study_dir)
        self.PID_ATTRIBUTE: str = "PID"
        self.LOCK_FILE_NAME: str = ".xpansion_locker"

        self.locker_file: Path = self.study_dir / self.LOCK_FILE_NAME

    def _set_study_dir(self, study_dir):
        if not study_dir.is_dir():
            raise StudyLocker.NotAValidDirectory(
                "%s is not a valid directory" % study_dir)
        self.study_dir = study_dir

    def lock(self):
        if self._is_locked():
            raise StudyLocker.Locked(
                f"{self.study_dir} is locked by an antares-xpansion process")

        self._lock()

    def unlock(self):
        if self._is_locked_by_another_process():
            raise StudyLocker.Locked(
                f"{self.study_dir} is locked by another xpansion instance")

        if self.locker_file.exists():
            self.locker_file.unlink()

    def _is_locked(self):
        if self.locker_file_missing_or_empty():
            return False

        pid = self._get_pid_from_locker_file()
        return psutil.pid_exists(pid)

    def _is_locked_by_another_process(self):
        if self.locker_file_missing_or_empty():
            return False

        pid = self._get_pid_from_locker_file()
        return not pid == os.getpid()

    def locker_file_missing_or_empty(self):
        return not self.locker_file.exists() or os.stat(self.locker_file).st_size == 0

    def _get_pid_from_locker_file(self):
        with open(self.locker_file, "r") as locker:
            lines = [line.rstrip() for line in locker.readlines()]
            non_blank_lines = [line for line in lines if line]
            if len(non_blank_lines) > 1:
                self.raise_corrupted_file_exception()
            else:
                pid = self._get_pid_from_non_blank_line(non_blank_lines[0])
        return pid

    def _get_pid_from_non_blank_line(self, line: str):
        if line.split('=')[0].strip() != self.PID_ATTRIBUTE:
            self.raise_corrupted_file_exception()
        try:
            pid = int(line.split('=')[1].strip())
        except ValueError:
            self.raise_corrupted_file_exception()
        return pid

    def raise_corrupted_file_exception(self):
        raise StudyLocker.CorruptedLockerFile(
            f"{self.study_dir} could not be locked, try to stop Xpansion instances and delete {self.locker_file}")

    def _lock(self):
        with open(self.locker_file, "w") as locker:
            locker.write(self.PID_ATTRIBUTE + " = " + str(os.getpid()))
        print(f"{self.study_dir} is locked")

    class NotAValidDirectory(Exception):
        pass

    class CorruptedLockerFile(Exception):
        pass

    class Locked(Exception):
        pass
