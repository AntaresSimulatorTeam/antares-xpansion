"""
    launches the execution of the antares xpansion c++ module
    Unified launcher: classic and trajectory modes
"""
import logging
import os
import sys
import resource
import subprocess
from datetime import datetime
from pathlib import Path

from antares_xpansion.__version__ import __version__, __revision__, __antares_simulator_version__
from antares_xpansion.config_file_parser import ConfigFileParser
from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.driver import XpansionDriver
from antares_xpansion.input_parser import InputParser
from antares_xpansion.log_utils import LogUtils
from antares_xpansion.logger import get_logger
from antares_xpansion.study_locker import StudyLocker
from antares_xpansion.xpansionConfig import XpansionConfig


def setup_core_dumps():
    """
    Enable core dumps temporarily for this run.
    Core dumps will be stored in /scratch/dumps
    """
    dumps_dir = Path("/scratch/dumps")
    try:
        # Create the dumps directory if it doesn't exist
        dumps_dir.mkdir(parents=True, exist_ok=True)

        # Set unlimited core dump size
        resource.setrlimit(resource.RLIMIT_CORE, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))

        # Configure kernel to save core dumps in /scratch/dumps with pattern
        try:
            subprocess.run(
                ["sysctl", "-w", "kernel.core_pattern=/scratch/dumps/core.%e.%p.%t"],
                check=False,
                capture_output=True
            )
        except Exception:
            # If sysctl fails (e.g., no root), continue anyway
            pass

        return True
    except Exception as e:
        print(f"Warning: Failed to setup core dumps: {e}", file=sys.stderr)
        return False


# Setup core dumps before starting the application
setup_core_dumps()

conf_file = Path(os.path.abspath(__file__)).parent / "config.yaml"
config_parser = ConfigFileParser(conf_file)
configuration_data = config_parser.get_config_parameters()

# Unified CLI: detect trajectory mode via --trajectory
TRAJECTORY_FLAG = "--trajectory"
if TRAJECTORY_FLAG in sys.argv:
    # Remove the flag so the trajectory parser can handle its own options
    sys.argv.remove(TRAJECTORY_FLAG)
    # Lazy imports to avoid hard dependency when not used
    from antares_xpansion.trajectory.args_parser_trajectory import TrajectoryArgsParser
    from antares_xpansion.trajectory.trajectory_config import TrajectoryConfig
    from antares_xpansion.trajectory.driver_trajectory import TrajectoryInvestmentDriver

    t_parser = TrajectoryArgsParser()
    t_params = t_parser.parse_args()

    # Pre-run logging
    step_info = {"step": "Pre Antares Trajectory"}
    logger = get_logger(__name__)
    logger.setLevel(logging.INFO)
    simple_message = {"simple": True}
    logger.info(
        "----------------------------------------------------------------", extra=simple_message)
    logger.info("Running Antares Xpansion (trajectory mode)... ", extra=step_info)
    logger.info(f"user: {LogUtils.user_name()}", extra=step_info)
    logger.info(f"hostname: {LogUtils.host_name()}", extra=step_info)
    logger.info(f"Xpansion version: {__version__}", extra=step_info)
    logger.info(f"Xpansion revision: {__revision__}", extra=step_info)
    logger.info(
        f"Antares Simulator version: {__antares_simulator_version__}", extra=step_info)
    logger.info(
        "----------------------------------------------------------------", extra=simple_message)

    start_time = datetime.now()

    # Trajectory mode works from input_root; no study locker on Antares output
    # But we still protect the directory from concurrent runs by using StudyLocker on input_root
    locker = StudyLocker(Path(t_params.input_root))
    locker.lock()

    t_config = TrajectoryConfig(t_params, configuration_data)
    t_driver = TrajectoryInvestmentDriver(t_config)
    try:
        t_driver.launch()

        end_time = datetime.now()
        xpansion_total_duration = end_time - start_time
        end_info = {"step": "Post Xpansion Trajectory"}
        logger.info(f"Xpansion (trajectory) duration : {xpansion_total_duration}", extra=end_info)
        logger.info("Xpansion (trajectory) Finished.", extra=end_info)
    except Exception as e:
        # Handle user-facing validation exceptions from trajectory input translator without stacktrace
        try:
            from antares_xpansion.trajectory.user_input_translation import UserInputTranslator

            user_exc_types = (
                UserInputTranslator.InvalidCandidates,
                UserInputTranslator.InvalidTreeStructure,
                UserInputTranslator.InvalidTrajectoryConstraint,
            )
        except Exception:
            user_exc_types = ()

        if isinstance(e, user_exc_types):
            print(f"Error: {e}", file=sys.stderr)
            locker.unlock()
            sys.exit(1)
        else:
            # Unexpected exception: re-raise to show full stacktrace for debugging
            locker.unlock()
            raise

    locker.unlock()
else:
    # Classic mode
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
