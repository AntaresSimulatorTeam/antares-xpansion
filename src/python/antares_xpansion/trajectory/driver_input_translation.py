from antares_xpansion.trajectory.user_input_translation import TrajectoryModule

from pathlib import Path


class InputTranslationDriver:
    """
    Drive the verification and translation of the user input file into
    the ```master_structrure.json``` intermediary file.
    """

    def __init__(self, input_file: Path, output_file: Path):
        self.input_file = input_file
        self.output_file = output_file
        pass

    def launch(self):
        translator = TrajectoryModule(self.input_file)
        translator.parse_trajectory_user_file()
        translator.run_all_verification()
        translator.print()
        translator.write_merger_json(self.output_file)
