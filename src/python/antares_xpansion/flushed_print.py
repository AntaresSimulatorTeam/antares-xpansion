
import functools

flushed_print = functools.partial(print, flush=True)