import argparse

from antares_xpansion.trajectory.trajectory_config import TrajectoryInputParameters

from pathlib import Path


class TrajectoryLauncherOptionsKeys:
    @staticmethod
    def step_key():
        return "step"

    @staticmethod
    def input_root_key():
        return "root"

    @staticmethod
    def input_file_key():
        return "input_file"

    @staticmethod
    def memory_key():
        return "memory"


class TrajectoryArgsParser:
    def __init__(self):
        self.parser = argparse.ArgumentParser()

        self._initialize_parser()

    def _initialize_parser(self):
        # Minimal arguments for now
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
            "--memory",
            action="store_true",
            dest=TrajectoryLauncherOptionsKeys.memory_key(),
            help="Execute the problem generation in memory",
        )

    def parse_args(self, args: list[str] = None) -> TrajectoryInputParameters:
        params = self.parser.parse_args(args)

        return TrajectoryInputParameters(
            step=params.step,
            input_root=Path(params.root),
            input_file=Path(params.input_file),
            memory=params.memory,
        )

    def _fill_default_values(self):
        pass
