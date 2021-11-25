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
        self.study_dir = None
        self._set_study_dir(study_dir)
        self.PID_ATTRIBUTE = "PID"
        self.LOCK_FILE_NAME = ".xpansion_locker"

        self.locker_file = self.study_dir / self.LOCK_FILE_NAME

    def _set_study_dir(self, study_dir):
        if not study_dir.is_dir():
            raise StudyLocker.NotAValidDirectory("%s is not a valid directory" % study_dir)
        self.study_dir = study_dir

    def lock(self):
        if self.is_unlocked():
            self._lock()
        else:
            raise StudyLocker.Locked(f"{self.study_dir} is locked by another xpansion instance")

    def is_unlocked(self):
        if not self.locker_file.exists():
            return True

        with open(self.locker_file, "r") as locker:
            lines = [line.rstrip() for line in locker.readlines()]
            non_blank_lines = [line for line in lines if line]
            if len(non_blank_lines) > 1:
                self.raise_corrupted_file_exception()
            elif len(non_blank_lines) == 0:
                return True
            else:
                pid = self._get_pid_from_lock_file(non_blank_lines[0])
                return not psutil.pid_exists(pid)

    def _get_pid_from_lock_file(self, line: str):
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
