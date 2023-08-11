
import functools
import logging
import sys


flushed_print = functools.partial(print, flush=True)


def get_logger(name, log_level = logging.INFO):

    class ConditionalFormatter(logging.Formatter):
        def format(self, record):
            if hasattr(record, 'simple') and record.simple:
                return record.getMessage()
            else:
                return logging.Formatter.format(self, record)

    logger = logging.getLogger(name)
    formatter = '[%(step)s][%(levelname)s %(asctime)s]  %(message)s'

    log_destination = sys.stderr
    if log_level <= logging.WARNING:
        log_destination = sys.stdout
    
    handler = logging.StreamHandler(log_destination)
    
    handler.setFormatter(ConditionalFormatter(
        formatter, datefmt="%d-%m-%Y %H:%M:%S"))
    logger.addHandler(handler)
    return logger


def step_logger(name, step=None, log_level=logging.INFO):
    extra = {"step": ""}
    if step is not None:
        extra["step"] = step
    logger = get_logger(name, log_level)
    logger.setLevel(log_level)

    return logging.LoggerAdapter(logger, extra)
