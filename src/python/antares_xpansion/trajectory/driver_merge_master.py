from dataclasses import dataclass
from pathlib import Path

from antares_xpansion.optimisation_keys import OptimisationKeys

import json
import subprocess
import sys
import os


@dataclass
class MergeMasterData:
    master_merger_exe: Path
    # Merge Master input files
    master_merger_info_file: Path
    nodal_lp_info_file: Path
    # Options file
    options_file: Path
    input_root: Path
    output_root: Path
    solver: str
    problems_format: str
    merged_master_name: str
    merged_structure_file: str


class MergeMasterDriver:
    """Drive the merge_master_mps executable"""

    class MergeMasterInputError(Exception):
        pass

    class MergeMasterExeError(Exception):
        pass

    class MergeMasterExecutionError(Exception):
        pass

    def __init__(self, data: MergeMasterData):
        self.merge_master_exe = data.master_merger_exe

        self.master_merger_info_file = data.master_merger_info_file
        self.nodal_lp_info_file = data.nodal_lp_info_file
        # Options
        self.options_file = data.options_file
        self.input_root = data.input_root
        self.output_root = data.output_root
        self.solver = data.solver
        self.problems_format = data.problems_format
        self.merged_master_name = data.merged_master_name
        self.merged_structure_file = data.merged_structure_file

        pass

    def _check_input_file_existence(self):
        # TODO : perhaps check more than simply existence ?
        if not self.master_merger_info_file.is_file():
            raise self.MergeMasterInputError(
                f"Master merge info file at : {self.master_merger_info_file} does not exist"
            )

        if not self.nodal_lp_info_file.is_file():
            raise self.MergeMasterInputError(
                f"Nodal lp info file at : {self.nodal_lp_info_file} does not exist"
            )

    def _generate_basic_merger_options_file(self):
        json_file = self.output_root / "out_merger.json"
        options = {
            OptimisationKeys.outpoutroot_key(): self.output_root.resolve().__str__(),
            OptimisationKeys.input_root_key(): self.input_root.resolve().__str__(),
            OptimisationKeys.json_file_key(): json_file.resolve().__str__(),
            OptimisationKeys.solver_name_key(): self.solver,
            OptimisationKeys.problems_format_key(): self.problems_format,
            OptimisationKeys.master_name_key(): self.merged_master_name,
            OptimisationKeys.structure_file_key(): self.merged_structure_file,
        }

        with open(self.options_file, "w") as f:
            json.dump(options, f, indent=4)

    def _get_master_merger_args(self):
        args: List[str] = []
        # Order is strict : <options_file> <master_merger_info.json> <nodal_lp_info.json>
        args.append(self.options_file)
        args.append(self.master_merger_info_file)
        args.append(self.nodal_lp_info_file)

        return args

    def _get_master_merger_command(self):
        if not self.merge_master_exe.is_file():
            raise self.MergeMasterExeError(
                f"merge_master_mps executable at : {self.merge_master_exe.__str__} not found"
            )

        command = [self.merge_master_exe]
        command.extend(self._get_master_merger_args())

        return command

    def _launch_merging(self):
        returned_l = subprocess.run(
            self._get_master_merger_command(),
            shell=False,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )

        if returned_l.returncode != 0:
            raise self.MergeMasterExecutionError(
                "ERROR: exited merge_master_mps with status %d" % returned_l.returncode
            )

    def launch(self):
        # Run the driver from the input root
        previous_dir = os.getcwd()
        os.chdir(self.input_root)

        self._check_input_file_existence()
        self._generate_basic_merger_options_file()
        self._launch_merging()

        os.chdir(previous_dir)
