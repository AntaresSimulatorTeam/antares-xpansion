"""
Controls the execution of the antares xpansion trajectory investment steps
"""

from pathlib import Path

from antares_xpansion.trajectory.args_parser_trajectory import (
    TrajectoryArgsParser,
)
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.trajectory.driver_trajectory import (
    TrajectoryInvestmentDriver,
)
from antares_xpansion.trajectory.trajectory_config import TrajectoryConfig

import os


conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
install_data = config_parser.get_config_parameters()

parser = TrajectoryArgsParser()
input_parameters = parser.parse_args()

config = TrajectoryConfig(input_parameters, install_data)

driver = TrajectoryInvestmentDriver(config)
driver.launch()
