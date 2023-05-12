"""
    launches the execution of the antares xpansion c++ module
"""
from datetime import datetime
import json
from pathlib import Path

from antares_xpansion.study_locker import StudyLocker
from antares_xpansion.input_parser import InputParser
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.__version__ import __version__, __antares_simulator_version__
from antares_xpansion.flushed_print import LOGLEVEL, XpansionLogger, flushed_print
from antares_xpansion.log_utils import LogUtils
import os


conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
configuration_data = config_parser.get_config_parameters()

parser = InputParser()
input_parameters = parser.parse_args()
logger = XpansionLogger()
logger.set_level(LOGLEVEL.INFO)
step = "Pre Antares"
logger.free_message(
    "----------------------------------------------------------------")
logger.message(step, "Running Antares Xpansion ... ")
logger.message(step, f"user: {LogUtils.user_name()}")
logger.message(step, f"hostname: {LogUtils.host_name()}")
logger.message(step, f"Xpansion version: {__version__}")
logger.message(
    step, f"antares simulator version: {__antares_simulator_version__}")
logger.free_message(
    "----------------------------------------------------------------")

start_time = datetime.now()

locker = StudyLocker(Path(input_parameters.data_dir))
locker.lock()
CONFIG = XpansionConfig(input_parameters, configuration_data)
config_loader = ConfigLoader(CONFIG)
DRIVER = XpansionDriver(config_loader)

DRIVER.launch()
end_time = datetime.now()

xpansion_total_duration = end_time - start_time

flushed_print('Xpansion duration : {}'.format(
    xpansion_total_duration))
flushed_print("Xpansion Finished.")
locker.unlock()
