from pathlib import Path

from antares_xpansion.benders_driver import BendersDriver
from antares_xpansion.full_run_driver import FullRunDriver
from antares_xpansion.problem_generator_driver import ProblemGeneratorDriver, ProblemGeneratorData

from test_problem_generator_driver import get_zipped_output

SUBPROCESS_RUN = "antares_xpansion.full_run_driver.subprocess.run"


class TestFullRunDriver:

    def setup_method(self):

        self.pb_gen_data = ProblemGeneratorData(keep_mps=False, additional_constraints="str", user_weights_file_path="",
                                                weight_file_name_for_lp="", lp_namer_exe_path=Path("lp.exe"), active_years=[1, 2])

        self.benders_driver_options_file = "options_file.json"

        self.full_run_exe = Path("full_run.exe")

    def test_sequential_command(self, tmp_path):

        output_path = tmp_path / "outputPath"
        self.create_problem_generation_files(output_path)
        output_path = get_zipped_output(output_path)
        benders_method = "benders"
        json_file_path = tmp_path / "file"
        json_file_path.touch()
        benders_keep_mps = False
        benders_oversubscribe = False
        benders_allow_run_as_root = False
        is_relaxed = False

        problem_generation = ProblemGeneratorDriver(self.pb_gen_data)

        problem_generation.set_output_path(output_path)
        problem_generation.create_lp_dir()
        benders_driver = BendersDriver(
            "benders.exe", "", self.benders_driver_options_file)
        full_run_driver = FullRunDriver(self.full_run_exe,
                                        problem_generation, benders_driver)
        full_run_driver.prepare_drivers(output_path, is_relaxed, json_file_path,
                                        benders_keep_mps=benders_keep_mps, benders_oversubscribe=benders_oversubscribe, benders_allow_run_as_root=benders_allow_run_as_root)
        xpansion_output_dir = output_path.parent / \
            (output_path.stem+"-Xpansion")
        expected_command = [self.full_run_exe, "--benders_options", self.benders_driver_options_file,
                            "-s", str(json_file_path), "-a", str(output_path), "-f", "integer", "-e", self.pb_gen_data.additional_constraints]

        command = full_run_driver.full_command()
        assert len(expected_command) == len(command)
        for item in expected_command:
            assert (item in command)

    def test_mpi_command(self, tmp_path):

        output_path = tmp_path / "outputPath"
        self.create_problem_generation_files(output_path)
        output_path = get_zipped_output(output_path)
        benders_method = "benders"
        json_file_path = tmp_path / "file"
        json_file_path.touch()
        benders_keep_mps = False
        benders_n_mpi = 3
        benders_oversubscribe = False
        benders_allow_run_as_root = False
        is_relaxed = False

        problem_generation = ProblemGeneratorDriver(self.pb_gen_data)
        problem_generation.set_output_path(output_path)
        problem_generation.create_lp_dir()

        benders_driver = BendersDriver(
            "benders.exe", "",  self.benders_driver_options_file)
        full_run_driver = FullRunDriver(self.full_run_exe,
                                        problem_generation, benders_driver)
        full_run_driver.prepare_drivers(output_path, is_relaxed, json_file_path,
                                        benders_keep_mps, benders_n_mpi, benders_oversubscribe, benders_allow_run_as_root)
        xpansion_output_dir = output_path.parent / \
            (output_path.stem+"-Xpansion")
        expected_command = [benders_driver.MPI_LAUNCHER, "-n", str(benders_n_mpi), self.full_run_exe, "--benders_options", self.benders_driver_options_file,
                            "-s", str(json_file_path), "-a", str(output_path), "-f", "integer", "-e", self.pb_gen_data.additional_constraints]

        command = full_run_driver.full_command()

        assert len(expected_command) == len(command)
        for item in expected_command:
            assert (item in command)

    def create_problem_generation_files(self, output_path: Path):

        output_path.mkdir()
        lp_path = output_path / "lp"
        lp_path.mkdir()
        area_file = output_path / "area-1.txt"
        area_file.touch()
        interco_file = output_path / "interco-1.txt"
        interco_file.touch()
