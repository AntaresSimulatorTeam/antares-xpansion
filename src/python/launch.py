"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver
import os

CONFIG = XpansionConfig(Path(os.path.abspath(__file__)).parent / "config.yaml")
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()

