"""
    launches the execution of the antares xpansion c++ module
"""
from pathlib import Path
from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver

CONFIG = XpansionConfig(Path.cwd() / "config.yaml")
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()

