
import functools

from .log_utils import LogUtils

flushed_print = functools.partial(print, flush=True)
log_message = functools.partial(
    print, f"[{LogUtils.user_name()}@{LogUtils.host_name()}] ", flush=True)
INFO_MSG = '<< INFO >>'
WARNING_MSG = '<< WARNING >>'
