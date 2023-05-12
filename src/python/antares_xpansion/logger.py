
import functools
import logging


flushed_print = functools.partial(print, flush=True)


@staticmethod
def get_logger(name):

    class ConditionalFormatter(logging.Formatter):
        def format(self, record):
            if hasattr(record, 'simple') and record.simple:
                return record.getMessage()
            else:
                return logging.Formatter.format(self, record)

    logger = logging.getLogger(name)
    formatter = '[%(step)s][%(levelname)s %(asctime)s]  %(message)s'
    handler = logging.StreamHandler()
    handler.setFormatter(ConditionalFormatter(
        formatter, datefmt="%d-%m-%Y %H:%M:%S"))
    logger.addHandler(handler)
    return logger
