"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path

from antares_xpansion.input_parser import InputParser
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
import os

conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
parser = InputParser()

CONFIG = XpansionConfig(parser.parse_args(), conf_file)
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()
