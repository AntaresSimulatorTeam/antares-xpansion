"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path

from antares_xpansion.input_parser import InputParser
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
import os

conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
configuration_data = config_parser.get_config_parameters()

parser = InputParser()

CONFIG = XpansionConfig(parser.parse_args(), configuration_data)
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()
