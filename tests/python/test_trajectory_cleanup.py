import os
from pathlib import Path

import yaml
from antares_xpansion.trajectory.driver_resolution import TrajectoryResolutionData, TrajectoryResolutionDriver

from file_creation import _create_empty_file_from_list


class TestTrajectoryCleanup:
    """
    Test that the trajectory resolution driver correctly cleans lp directories for all nodes
    """

    def test_clean_all_nodes_lp_directories(self, tmp_path):
        """
        Test that _clean_all_nodes_lp_directories correctly identifies and cleans
        the lp directories of all trajectory nodes
        """
        # Setup: Create a fake trajectory structure with multiple nodes
        input_root = tmp_path / "input_root"
        input_root.mkdir()

        # Create fake studies for each node
        node_2030_study = tmp_path / "node_2030_study"
        node_2040_study = tmp_path / "node_2040_study"

        for study_path in [node_2030_study, node_2040_study]:
            study_path.mkdir()
            output_dir = study_path / "output"
            output_dir.mkdir()

            # Create a fake simulation output directory
            sim_output = output_dir / "20250101-1200eco"
            sim_output.mkdir()

            # Create the lp directory with files to be cleaned
            lp_dir = sim_output / "lp"
            lp_dir.mkdir()

            # Create files that should be removed
            files_to_remove = ["problem1.mps", "problem2.lp", "subproblem.mps"]
            _create_empty_file_from_list(lp_dir, files_to_remove)

            # Create files that should be kept (master files)
            files_to_keep = ["master.mps", "master_last_iteration.mps"]
            _create_empty_file_from_list(lp_dir, files_to_keep)

        # Create user input file
        user_input_file = input_root / "input-trajectory.yaml"
        user_data = {
            "global": {
                "formulation": "relaxed",
                "discount_rate": 0.064,
                "first_investment_year": 2030,
                "end_of_horizon": 2060,
                "scaling": 1,
                "studies": {
                    "2030": str(node_2030_study),
                    "2040": str(node_2040_study),
                }
            }
        }

        with open(user_input_file, "w") as f:
            yaml.dump(user_data, f)

        # Create a minimal TrajectoryResolutionData object
        # We only need user_input_file and input_root for the cleanup test
        res_data = TrajectoryResolutionData(
            benders_exe=Path("/dummy/benders"),
            frontal_exe=Path("/dummy/frontal"),
            outer_loop_exe=Path("/dummy/outer_loop"),
            mpi_exe=Path("/dummy/mpiexec"),
            input_root=input_root,
            root_study=node_2030_study,
            json_output_file=tmp_path / "out.json",
            benders_options_file=tmp_path / "options.json",
            merged_weights_file=tmp_path / "weights.txt",
            output_folder=tmp_path / "output",
            user_input_file=user_input_file,
            master_name="master",
            structure_file="structure.txt",
            solver="Xpress",
            problems_format="MPS",
            cache_problems=False,
            method="benders",
            n_mpi=1,
            oversubscribe=False,
            allow_run_as_root=False,
            master_formulation="relaxed",
        )

        # Create the driver and test the cleanup method
        driver = TrajectoryResolutionDriver(res_data)
        driver._clean_all_nodes_lp_directories()

        # Verify that files were cleaned in both nodes
        for study_path in [node_2030_study, node_2040_study]:
            sim_output = study_path / "output" / "20250101-1200eco"
            lp_dir = sim_output / "lp"

            # Check that .mps and .lp files (except master files) were removed
            remaining_files = os.listdir(lp_dir)

            # Files that should have been removed
            assert "problem1.mps" not in remaining_files
            assert "problem2.lp" not in remaining_files
            assert "subproblem.mps" not in remaining_files

            # Files that should still exist
            assert "master.mps" in remaining_files
            assert "master_last_iteration.mps" in remaining_files

    def test_read_node_to_studies(self, tmp_path):
        """
        Test that _read_node_to_studies correctly reads the user input file
        """
        input_root = tmp_path / "input_root"
        input_root.mkdir()

        # Create user input file with relative and absolute paths
        user_input_file = input_root / "input-trajectory.yaml"
        node_2030_study = tmp_path / "node_2030_study"
        node_2030_study.mkdir()

        user_data = {
            "global": {
                "formulation": "relaxed",
                "discount_rate": 0.064,
                "first_investment_year": 2030,
                "end_of_horizon": 2060,
                "scaling": 1,
                "studies": {
                    "2030": "./relative_path",
                    "2040": str(node_2030_study),  # Absolute path
                }
            }
        }

        with open(user_input_file, "w") as f:
            yaml.dump(user_data, f)

        # Create a minimal TrajectoryResolutionData object
        res_data = TrajectoryResolutionData(
            benders_exe=Path("/dummy/benders"),
            frontal_exe=Path("/dummy/frontal"),
            outer_loop_exe=Path("/dummy/outer_loop"),
            mpi_exe=Path("/dummy/mpiexec"),
            input_root=input_root,
            root_study=node_2030_study,
            json_output_file=tmp_path / "out.json",
            benders_options_file=tmp_path / "options.json",
            merged_weights_file=tmp_path / "weights.txt",
            output_folder=tmp_path / "output",
            user_input_file=user_input_file,
            master_name="master",
            structure_file="structure.txt",
            solver="Xpress",
            problems_format="MPS",
            cache_problems=False,
            method="benders",
            n_mpi=1,
            oversubscribe=False,
            allow_run_as_root=False,
            master_formulation="relaxed",
        )

        driver = TrajectoryResolutionDriver(res_data)
        node_to_studies = driver._read_node_to_studies()

        # Verify that paths were read correctly
        assert "2030" in node_to_studies
        assert "2040" in node_to_studies

        # Relative path should be resolved relative to input_root
        assert node_to_studies["2030"] == input_root / "./relative_path"

        # Absolute path should remain absolute
        assert node_to_studies["2040"] == node_2030_study
