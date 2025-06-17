from antares_xpansion.trajectory.user_input_translation import TrajectoryModule

from pathlib import Path

import sys


class InputTranslationDriver:
    """
    Drive the verification and translation of the user input file into
    the ```master_merger_info.json``` intermediary file.
    """

    class InvalidPythonVersion(Exception):
        pass

    class InvalidRootStudyPathError(Exception):
        pass

    def __init__(self, input_file: Path, output_file: Path):
        self.input_file = input_file
        self.output_file = output_file
        self.translator = TrajectoryModule(self.input_file)
        # Only parse the input once
        self.input_parsed = False
        pass

    def _parse_input(self):
        # Only parse the input once
        if not sys.version_info >= (3, 7):
            # Note that we did not check that it works with 3.7, we only know for sure that it doesn't under 3.6
            raise self.InvalidPythonVersion(
                "User input translation only works with Pydantic 2 and python 3.7 and above"
            )
        if not self.input_parsed:
            self.translator.parse_trajectory_user_file()
            self.input_parsed = True

    def get_root_study(self, input_root: Path) -> Path:
        """
        Returns the absolute path to the root study.
        Takes in a path to the root folder containing all the studies.
        """
        # self._parse_input()
        path = input_root / self.translator.get_root_study()
        if not path.is_dir():
            raise self.InvalidRootStudyPathError(
                f"The root study should be found at '{path.resolve().__str__()}', but this directory does not exist"
            )
        return path

    def launch(self):
        raise self.InvalidPythonVersion(
            "User input translation only works with Pydantic 2 and python 3.7 and above"
        )
        self._parse_input()
        self.translator.run_all_verification()
        self.translator.print()
        self.translator.write_merger_json(self.output_file)
