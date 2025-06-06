from pathlib import Path
from dataclasses import dataclass

import yaml

from antares_xpansion.trajectory.user_input_keys import TrajectoryInputKeys as InKeys


@dataclass
class MultipleProblemGenerationData:
    exe_path: Path
    user_input_file: Path
    memory: bool


# Question : which component generates the 3 differents
# study_input_file
# weights_info_file
# additional_constraints_info_file
# that need to be passed to the C++ executable ?
class MultipleProblemGenerationDriver:
    def __init__(self, data: MultipleProblemGenerationData):
        self.exe_path = data.exe_path
        self.user_input_file = data.user_input_file
        self.memory = data.memory

        self.node_studies: dict[str, Path]
        self.node_weights_file: dict[str, Path]
        self.node_additional_constraitns: dict[str, Path]

    def _prepare_input_files(self):
        """
        Load the user input file and load into memory the list of studies.
        Load each study to get the path of the possible weights (resp. additional constraints) file.
        """
        with open(self.user_input_file) as file:
            user_data = yaml.full_load(file)
            self.node_studies = user_data[InKeys.global_key()][InKeys.studies_key()]

        # Loop over the studies.
        # TBA : we need a ConfigLoader object, but to create one we need all the study / input / config parameters :(
