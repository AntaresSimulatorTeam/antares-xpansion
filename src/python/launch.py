"""
    launches the execution of the antares xpansion c++ module
"""
from datetime import datetime
import json
from pathlib import Path
from antares_xpansion.log_utils import LogUtils

from antares_xpansion.study_locker import StudyLocker
from antares_xpansion.input_parser import InputParser
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.__version__ import __version__, __antares_simulator_version__
from antares_xpansion.flushed_print import flushed_print
import os


conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
configuration_data = config_parser.get_config_parameters()

parser = InputParser()
input_parameters = parser.parse_args()

flushed_print(
    "----------------------------------------------------------------")
flushed_print("Running Antares Xpansion ... ")
flushed_print(f"user: {LogUtils.user_name()}")
flushed_print(f"hostname: {LogUtils.host_name()}")
flushed_print(f"Xpansion version: {__version__}")
flushed_print(f"antares simulator version: {__antares_simulator_version__}")
flushed_print(
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
