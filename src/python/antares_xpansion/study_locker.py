import os
from pathlib import Path
import psutil

class BaseException(Exception):
    pass

class StudyLocker:
    """
        class resposible for locking a study already launched
    """
    def __init__(self, study_dir: Path) -> None:
        if not study_dir.is_dir():
            raise StudyLocker.NotAValidDirectory("%s is not a valid directory"%study_dir)
        self.study_dir = study_dir
        self.PID_ATTRIBUT = "PID"
        self.LOCK_FILE_NAME = ".xpansion_locker" 
        
        self.locker_file = self.study_dir / self.LOCK_FILE_NAME
    
    def lock(self):
        if self.unlocked():
            self._lock()
        else :
            raise StudyLocker.Locked(f"{self.study_dir} is locked by another xpansion instance")

    def unlocked(self):
        if not self.locker_file.exists():
            return True

        with open(self.locker_file, "r") as locker :
            lines = locker.readlines()
            lines = [line.rstrip() for line in lines] # All lines including the blank ones
            lines = [line for line in lines if line]  # exclude blank lines
            if len(lines) > 1:
                raise StudyLocker.CorruptedLockerFile(f"{self.study_dir} could not be locked, try to stop Xpansion instances and delete {self.locker_file}")
            elif len(lines) == 0 :
                return True
            else :
                pid = self._check_lock_info_consistency(lines[0])
                return not psutil.pid_exists(pid)
                
    def _check_lock_info_consistency(self, line: str):
        if line.split('=')[0].strip() != self.PID_ATTRIBUT :
            raise StudyLocker.CorruptedLockerFile(f"{self.study_dir} could not be locked, try to stop Xpansion instances and delete {self.locker_file}")
        pid = line.split('=')[1].strip()
        if not pid.isdigit():
            raise StudyLocker.CorruptedLockerFile(f"{self.study_dir} could not be locked, try to stop Xpansion instances and delete {self.locker_file}")

        return int(pid)
    
    def _lock(self):
        with open(self.locker_file, "w") as locker :
            locker.write(self.PID_ATTRIBUT+ " = " +str(os.getpid()) )
        print(f"{self.study_dir} is locked")

    class NotAValidDirectory(BaseException):
        pass

    class CorruptedLockerFile(BaseException):
        pass
    class Locked(BaseException):
        pass

    
