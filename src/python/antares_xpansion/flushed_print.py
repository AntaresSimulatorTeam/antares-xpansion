
from enum import Enum
import functools
from datetime import datetime


def now_datetime_formatted():
    now = datetime.now()

    timestamp = now.strftime("%d-%m-%Y %H:%M:%S")
    return timestamp


flushed_print = functools.partial(print, flush=True)


@staticmethod
def prefix_log(step, log_level):
    return f"[{step}][{log_level} {now_datetime_formatted()}] "


INFO_MSG = '<< INFO >>'
WARNING_MSG = '<< WARNING >>'


class LOGLEVEL(Enum):
    INFO = 1,
    WARNING = 2


class XpansionLogger:
    def __init__(self) -> None:
        self.level = LOGLEVEL.INFO

    def info(self, step, message):
        flushed_print(prefix_log(step, INFO_MSG) + message)

    def warning(self, step, message):
        flushed_print(prefix_log(step, WARNING_MSG) + message)

    def set_level(self, level: LOGLEVEL):
        self.level = level

    def message(self, step, message):
        flushed_print(prefix_log(
            step, XpansionLogger.to_str(self.level))+message)

    def free_message(self, message):
        flushed_print(message)

    @staticmethod
    def to_str(level: LOGLEVEL):
        if level == LOGLEVEL.INFO:
            return INFO_MSG
        elif level == LOGLEVEL.WARNING:
            return WARNING_MSG
