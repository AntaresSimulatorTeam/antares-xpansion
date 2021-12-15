"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path

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
flushed_print(f"Xpansion version: {__version__}")
flushed_print(f"antares simulator version: {__antares_simulator_version__}")
flushed_print(
    "----------------------------------------------------------------")


locker = StudyLocker(Path(input_parameters.data_dir))
locker.lock()
CONFIG = XpansionConfig(input_parameters, configuration_data)
config_loader = ConfigLoader(CONFIG)
DRIVER = XpansionDriver(config_loader)

DRIVER.launch()
flushed_print("Xpansion Finished.")

locker.unlock()
