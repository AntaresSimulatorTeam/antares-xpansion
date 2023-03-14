
import functools

from antares_xpansion.log_utils import LogUtils

flushed_print = functools.partial(print, flush=True)
log_message = functools.partial(
    flushed_print, f"[{LogUtils.user_name()}@{LogUtils.host_name()}] ")
INFO_MSG = '<< INFO >>'
WARNING_MSG = '<< WARNING >>'
