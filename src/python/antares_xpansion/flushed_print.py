
import functools

flushed_print = functools.partial(print, flush=True)
INFO_MSG = '<< INFO >>'
WARNING_MSG = '<< WARNING >>'
