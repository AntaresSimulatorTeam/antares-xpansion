import argparse

from antares_xpansion.trajectory.launcher_options_trajectory import (
    TrajectoryLauncherOptionsKeys,
)
from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters


class TrajectoryArgsParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()

        self._initialize_parser()

    def _initialize_parser(self):
        # Minimal arguments for now
        self.parser.add_argument(
            "--step",
            dest=TrajectoryLauncherOptionsKeys.step_key(),
            choices=[
                "full",
                "input_translation",
                "problem_generation",
                "merge_master",
                "merge_weights",
                "resolution",
            ],
            help='Step to execute ("full", "input_translation", "problem_generation", "merge_master", "merge_weights", "resolution")',
            required=True,
        )
        self.parser.add_argument(
            "--input-root",
            dest=TrajectoryLauncherOptionsKeys.input_root_key(),
            help="Input root, folder containing all the studies in the tree.",
            required=True,
        )
        self.parser.add_argument(
            "--input-file",
            dest=TrajectoryLauncherOptionsKeys.input_file_key(),
            help="User data input file",
            required=True,
        )

    def parse_args(self, args: list[str] = None) -> TrajectoryInputParameters:
        params = self.parser.parse_args(args)

        return TrajectoryInputParameters(
            step=params.step, input_root=params.root, input_file=params.input_file
        )

    def _fill_default_values(self):
        pass
