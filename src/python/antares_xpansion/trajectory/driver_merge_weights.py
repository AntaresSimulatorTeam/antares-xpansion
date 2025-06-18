from dataclasses import dataclass
from pathlib import Path

import subprocess
import sys


@dataclass
class MergeWeightsData:
    merge_weights_exe: Path
    master_merger_info_file: Path
    nodal_lp_info_file: Path
    output_file: Path


class MergeWeightsDriver:
    """Drive the merge_weights_trajectory executable"""

    class MergeWeightsInputError(Exception):
        pass

    class MergeWeightsExeError(Exception):
        pass

    class MergeWeightsExecutionError(Exception):
        pass

    def __init__(self, data: MergeWeightsData):
        self.merge_weights_exe = data.merge_weights_exe
        self.master_merger_info_file = data.master_merger_info_file
        self.nodal_lp_info_file = data.nodal_lp_info_file
        self.output_file = data.output_file

    def _check_input_file_existence(self):
        # TODO : perhaps check more than simply existence ?
        if not self.master_merger_info_file.is_file():
            raise self.MergeWeightsInputError(
                f"Master merge info file at : {self.master_merger_info_file} does not exist"
            )

        if not self.nodal_lp_info_file.is_file():
            raise self.MergeWeightsInputError(
                f"Nodal lp info file at : {self.nodal_lp_info_file} does not exist"
            )

    def _get_merge_weights_args(self):
        args: List[str] = []
        # Args are fixed order : <master_merger_info.json> <nodal_lp_info.json> <output_file>
        args.append(self.master_merger_info_file)
        args.append(self.nodal_lp_info_file)
        args.append(self.output_file)

        return args

    def _get_merge_weights_command(self):
        if not self.merge_weights_exe.is_file():
            raise self.MergeWeightsExeError(
                f"merge_weights_trajecotry executable at : {self.merge_weights_exe.__str__} not found"
            )

        command = [self.merge_weights_exe]
        command.extend(self._get_merge_weights_args())

        return command

    def _launch_weights_merge(self):
        returned_l = subprocess.run(
            self._get_merge_weights_command(),
            shell=False,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )

        if returned_l.returncode != 0:
            raise self.MergeWeightsExecutionError(
                "ERROR: exited merge_weights_trajectory with status %d"
                % returned_l.returncode
            )

    def launch(self):
        self._check_input_file_existence()
        self._launch_weights_merge()
