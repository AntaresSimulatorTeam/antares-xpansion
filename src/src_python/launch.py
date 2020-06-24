"""
    launches the execution of the antares xpansion c++ module
"""

from antares_xpansion.xpansionConfig import XpansionConfig
from antares_xpansion.driver import XpansionDriver

CONFIG = XpansionConfig()
DRIVER = XpansionDriver(CONFIG)

DRIVER.launch()

#MY_LP_PATH = DRIVER.generate_mps_files()
# my_lp_path = 'D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco\\lp'
# DRIVER.set_options('D:\\repo\\these-blanchot-lp-namer\\test_case\\output\\20200214-1622eco')
# DRIVER.launch_optimization(MY_LP_PATH, CONFIG.BENDERS_SEQUENTIAL)
# DRIVER.launch_optimization(MY_LP_PATH, CONFIG.MERGE_MPS)
