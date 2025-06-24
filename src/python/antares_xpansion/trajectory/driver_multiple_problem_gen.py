from pathlib import Path
from dataclasses import dataclass

import yaml
import subprocess
import sys
import os

from antares_xpansion.trajectory.user_input_keys import TrajectoryInputKeys as InKeys
from antares_xpansion.xpansionConfig import XpansionConfigConstants
from antares_xpansion.config_loader import XpansionSettingsReader

from typing import List, Dict


@dataclass
class MultipleProblemGenerationData:
    """
    Expects paths to be given as full paths to the files and executables
    Takes in the input root to write the paths to the respective studies
    / weights files / additional constraints file as full absolute paths.
    """

    exe_path: Path
    input_root: Path
    user_input_file: Path
    memory: bool
    # Input files
    mpg_input: Path
    mpg_weights: Path
    mpg_constraints: Path
    # Output file
    mpg_nodal_lp_info_file: Path


class MultipleProblemGenerationDriver:
    """
    ONLY MEMORY MODE IS IMPLEMENTED FOR NOW
    """

    class MultipleProblemGenerationExeError(Exception):
        pass

    class MultipleProblemGenerationExecutionError(Exception):
        pass

    def __init__(self, data: MultipleProblemGenerationData):
        self.exe_path = data.exe_path
        self.input_root = data.input_root
        self.user_input_file = data.user_input_file
        self.memory = data.memory

        # Input files
        self.input_file = data.mpg_input
        self.node_to_custom_weights_file_file = data.mpg_weights
        self.node_to_add_constraints_file_file = data.mpg_constraints

        # Output file
        self.nodal_lp_info_file = data.mpg_nodal_lp_info_file

        self.node_to_studies: Dict[str, Path] = {}
        self.node_to_weights_file: Dict[str, Path] = {}
        self.node_to_additional_constraints: Dict[str, Path] = {}

        # Only use one mode of formulation : either all relaxed or all integer
        # Get this value from the user file in _read_data_and_prepare_input_files
        self.formulation: str = "relaxed"

    def _read_data_and_prepare_input_files(self):
        """
        Load the user input file and load into memory the list of studies.
        Read the choice of formulation, either "relaxed" or "integer"
        Load each study to get the path of the possible weights (resp. additional constraints) file.
        """
        with open(self.user_input_file, encoding="utf-8") as file:
            user_data = yaml.full_load(file)
            self.formulation = user_data[InKeys.global_key()][InKeys.formulation_key()]
            studies: Dict[str, str] = user_data[InKeys.global_key()][
                InKeys.studies_key()
            ]
            for node, pathstr in studies.items():
                path = Path(pathstr)
                # We write the paths relative to the input root.
                self.node_to_studies[node] = path

        # Loop over the studies.
        config_defaults = XpansionConfigConstants()
        config_defaults._initialize_default_values()

        # We have to work at the input root
        previous_dir = os.getcwd()
        os.chdir(self.input_root)

        for node, path in self.node_to_studies.items():
            print(f"Reading file {path.__str__()}")
            reader = XpansionSettingsReader(path, config_defaults)
            weights_file = reader.weights_file_path()
            if weights_file != "":
                self.node_to_weights_file[node] = Path(weights_file)
            constraints_file = reader.additional_constraints()
            if constraints_file != "":
                self.node_to_additional_constraints[node] = Path(constraints_file)

        os.chdir(previous_dir)

    @staticmethod
    def _write_dict_to_file(dict: Dict[str, Path], filename: Path):
        lines: List[str] = []
        for node, path in dict.items():
            lines.append(f"{node} {str(path)}")
        with open(filename, "w") as f:
            f.write("\n".join(lines))

    def _write_input_files(self):
        # Note that only memory mode is implemented for now.
        # In archive mode, the dict we want to write is : node -> last_antares_output_archive
        # This is not implemented for now
        if self.memory:
            self._write_dict_to_file(self.node_to_studies, self.input_file)
        else:
            raise NotImplementedError(
                "Non memory mode is not yet implemented in the MultipleProblemGeneration driver"
            )

        # Only write the weights (resp constraints) file if they are not empty
        # i.e. if one study at least has a custom weights file (resp additional constraints)
        if self.node_to_weights_file:
            self._write_dict_to_file(
                self.node_to_weights_file, self.node_to_custom_weights_file_file
            )
        if self.node_to_additional_constraints:
            self._write_dict_to_file(
                self.node_to_additional_constraints,
                self.node_to_add_constraints_file_file,
            )

    def _get_mpg_args(self):
        args: List[str] = []
        if self.memory:
            args.extend(["--study", self.input_file])
        else:
            # TODO : add non memory implementation
            pass
        # Formulation
        args.extend(["-f", self.formulation])
        # NodeToCustomWeights file
        if self.node_to_weights_file:
            args.extend(["-w", self.node_to_custom_weights_file_file])
        # NodeToAdditionalConstraints file
        if self.node_to_additional_constraints:
            args.extend(["-e", self.node_to_add_constraints_file_file])

        # Output file
        args.extend(["--nodal-file", self.nodal_lp_info_file])

        # Input root
        args.extend(["--input-root", self.input_root])

        return args

    def _get_mpg_command(self):
        if not self.exe_path.is_file():
            raise self.MultipleProblemGenerationExeError(
                f"MPG exe : {self.exe_path} not found"
            )
        command = [self.exe_path]
        command.extend(self._get_mpg_args())
        return command

    def _launch_executable(self):
        returned_l = subprocess.run(
            self._get_mpg_command(),
            shell=False,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )
        if returned_l.returncode != 0:
            raise self.MultipleProblemGenerationExecutionError(
                "ERROR: exited multiple_problem_generation with status %d"
                % returned_l.returncode
            )

    def launch(self):
        # Change working directory to input root.
        previous_dir = os.getcwd()
        os.chdir(self.input_root)
        # Run the driver
        self._read_data_and_prepare_input_files()
        self._write_input_files()
        self._launch_executable()

        os.chdir(previous_dir)

        return
