from antares_xpansion.trajectory.user_input_translation import UserInputTranslator

from pathlib import Path
import os


class InputTranslationDriver:
    """
    Drive the verification and translation of the user input file into
    the ```master_merger_info.json``` intermediary file.
    """

    class InvalidRootStudyPathError(Exception):
        pass

    def __init__(self, input_root: Path, input_file: Path, output_file: Path):
        # Both are probably absolute so equivalent to self.input_file = input_file
        self.input_file = input_root / input_file
        self.input_root = input_root
        # Output file is a path relative to the input root.
        self.output_file = output_file
        self.translator = UserInputTranslator(self.input_file)
        # Only parse the input once
        self.input_parsed = False
        pass

    def _parse_input(self):
        # Only parse the input once
        if not self.input_parsed:
            self.translator.parse_trajectory_user_file()
            self.input_parsed = True

    def get_root_study(self, input_root: Path) -> Path:
        """
        Returns the absolute path to the root study.
        Takes in a path to the root folder containing all the studies.
        """
        self._parse_input()
        path = input_root / self.translator.get_root_study()
        if not path.is_dir():
            raise self.InvalidRootStudyPathError(
                f"The root study should be found at '{path.resolve().__str__()}', but this directory does not exist"
            )
        return path

    def get_master_formulation(self):
        """
        Returns the formulation parameter entered by the user, has to be either 'integer' or 'relaxed'
        """
        self._parse_input()
        formulation = self.translator.global_data.formulation.value

        return formulation

    def launch(self):
        previous_dir = os.getcwd()
        os.chdir(self.input_root)

        # Launch the parsing and writing of the intermediary file
        self._parse_input()
        self.translator.run_all_verification()
        self.translator.print()
        self.translator.write_merger_json(self.output_file)

        os.chdir(previous_dir)
