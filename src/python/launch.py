"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
import os

conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
CONFIG = XpansionConfig(conf_file)
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()
