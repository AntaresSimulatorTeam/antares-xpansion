"""
    launches the execution of the antares xpansion c++ module
"""
from datetime import datetime
import json
import logging
from pathlib import Path
from antares_xpansion.input_parser import InputParser

from antares_xpansion.study_locker import StudyLocker
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.__version__ import __version__,__revision__, __antares_simulator_version__
from antares_xpansion.logger import get_logger, step_logger
from antares_xpansion.log_utils import LogUtils
import os


conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
configuration_data = config_parser.get_config_parameters()

parser = InputParser()
input_parameters = parser.parse_args()

step_info = {"step": "Pre Antares"}
logger = get_logger(__name__)
logger.setLevel(logging.INFO)
simple_message = {"simple": True}
logger.info(
    "----------------------------------------------------------------", extra=simple_message)
logger.info("Running Antares Xpansion ... ", extra=step_info)
logger.info(f"user: {LogUtils.user_name()}", extra=step_info)
logger.info(f"hostname: {LogUtils.host_name()}", extra=step_info)
logger.info(f"Xpansion version: {__version__}", extra=step_info)
logger.info(f"Xpansion revision: {__revision__}", extra=step_info)
logger.info(
    f"Antares Simulator version: {__antares_simulator_version__}", extra=step_info)
logger.info(
    "----------------------------------------------------------------", extra=simple_message)

start_time = datetime.now()

locker = StudyLocker(Path(input_parameters.data_dir))
locker.lock()
CONFIG = XpansionConfig(input_parameters, configuration_data)
config_loader = ConfigLoader(CONFIG)
DRIVER = XpansionDriver(config_loader)

DRIVER.launch()
end_time = datetime.now()

xpansion_total_duration = end_time - start_time
end_info = {"step": "Post Xpansion"}

logger.info(f"Xpansion duration : {xpansion_total_duration}", extra=end_info)
logger.info("Xpansion Finished.", extra=end_info)
locker.unlock()
