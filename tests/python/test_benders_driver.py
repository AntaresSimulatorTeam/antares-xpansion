import pytest
import os, sys
from pathlib import Path
from unittest.mock import patch

from antares_xpansion.benders_driver import BendersDriver, BendersDriverData
MOCK_SUBPROCESS_RUN = "antares_xpansion.benders_driver.subprocess.run"
MOCK_SYS = "antares_xpansion.benders_driver.sys"
class TestBendersDriver:

    def setup_method(self):
        self.empty_benders_driver_data  = BendersDriverData(install_dir = Path(""),
                                                            BENDERS_MPI = "",
                                                            method = "",
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = "",
                                                            n_mpi = 0,
                                                            keep_mps = "")


    def test_driver_with_empty_data(self):
        benders_driver = BendersDriver(self.empty_benders_driver_data)
        
        assert benders_driver.install_dir == self.empty_benders_driver_data.install_dir 
        assert benders_driver.BENDERS_MPI == self.empty_benders_driver_data.BENDERS_MPI
        assert benders_driver.method == self.empty_benders_driver_data.method
        assert benders_driver.MERGE_MPS == self.empty_benders_driver_data.MERGE_MPS
        assert benders_driver.BENDERS_SEQUENTIAL == self.empty_benders_driver_data.BENDERS_SEQUENTIAL
        assert benders_driver.n_mpi == self.empty_benders_driver_data.n_mpi
        assert benders_driver.keep_mps == self.empty_benders_driver_data.keep_mps

    def test_driver_with_data(self):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        benders_driver = BendersDriver(benders_driver_data)
        
        assert benders_driver.install_dir == benders_driver_data.install_dir 
        assert benders_driver.BENDERS_MPI == benders_driver_data.BENDERS_MPI
        assert benders_driver.method == benders_driver_data.method
        assert benders_driver.MERGE_MPS == benders_driver_data.MERGE_MPS
        assert benders_driver.BENDERS_SEQUENTIAL == benders_driver_data.BENDERS_SEQUENTIAL
        assert benders_driver.n_mpi == benders_driver_data.n_mpi
        assert benders_driver.keep_mps == benders_driver_data.keep_mps

    def test_mpi_options(self):
        benders_driver = BendersDriver(self.empty_benders_driver_data)

        if sys.platform.startswith("win32"):
            assert benders_driver.MPI_LAUNCHER == "mpiexec"
            assert benders_driver.MPI_N == "-n"
        elif sys.platform.startswith("linux"):
            benders_driver.MPI_LAUNCHER = "mpirun"
            benders_driver.MPI_N = "-np"
    
    def test_lp_path(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        lp_path = tmp_path / "lp" 
        os.mkdir(lp_path)
        benders_driver = BendersDriver(benders_driver_data)
        benders_driver.set_simulation_output_path(tmp_path)
        assert benders_driver.get_lp_path() == lp_path
    
    def test_non_existing_output_path(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)

        benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(BendersDriver.BendersOutputPathError):
            benders_driver.launch(tmp_path / "i_dont_exist")
    
    def test_empty_output_path(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)

        benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(BendersDriver.BendersLpPathError):
            benders_driver.launch(tmp_path)

    
    def test_method_must_be_defined_before_launch(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        lp_path = tmp_path / "lp" 
        os.mkdir(lp_path)
        benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(BendersDriver.BendersSolverError):
            benders_driver.launch(tmp_path)
            
    
    def test_method_both_is_not_accepted(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "Both",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        lp_path = tmp_path / "lp" 
        os.mkdir(lp_path)
        benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(SystemExit):
            benders_driver.launch(tmp_path)
            
    
    def test_method_both_is_not_accepted(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "both",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        lp_path = tmp_path / "lp" 
        os.mkdir(lp_path)
        benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(SystemExit):
            benders_driver.launch(tmp_path)
                        
    def test_benders_cmd_mpibenders(self, tmp_path):
        my_benders_mpi = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        exe_path =  os.path.normpath(os.path.join(my_install_dir, my_benders_mpi))

        if sys.platform.startswith("win32"):
            MPI_LAUNCHER = "mpiexec"
            MPI_N = "-n"
        elif sys.platform.startswith("linux"):
            MPI_LAUNCHER = "mpirun"
            MPI_N = "-np"
        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = True)
        
        benders_driver = BendersDriver(benders_driver_data)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)
        assert benders_driver.get_solver() == my_benders_mpi
        with patch(MOCK_SUBPROCESS_RUN, autospec = True) as run_function :
            expected_cmd = [MPI_LAUNCHER, MPI_N, str(my_n_mpi), exe_path, "options.txt" ]
            run_function.return_value.returncode = 0
            benders_driver.launch(simulation_output_path)
            assert run_function.call_args.args[0] == expected_cmd
                        
    def test_benders_cmd_sequential(self, tmp_path):
        my_sequential = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        exe_path =  os.path.normpath(os.path.join(my_install_dir, my_sequential))

        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = "test",
                                                            method = "sequential",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = my_sequential,
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = True)
        
        benders_driver = BendersDriver(benders_driver_data)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)
        assert benders_driver.get_solver() == my_sequential
        with patch(MOCK_SUBPROCESS_RUN, autospec = True) as run_function :
            expected_cmd = [exe_path, "options.txt" ]
            run_function.return_value.returncode = 0
            benders_driver.launch(simulation_output_path)
            assert run_function.call_args.args[0] == expected_cmd
                        
    def test_benders_cmd_sequential(self, tmp_path):
        my_merges_mps = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        exe_path =  os.path.normpath(os.path.join(my_install_dir, my_merges_mps))

        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = "test",
                                                            method = "mergeMPS",
                                                            MERGE_MPS  = my_merges_mps,
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = True)
        
        benders_driver = BendersDriver(benders_driver_data)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)
        assert benders_driver.get_solver() == my_merges_mps
        with patch(MOCK_SUBPROCESS_RUN, autospec = True) as run_function :
            expected_cmd = [exe_path, "options.txt" ]
            run_function.return_value.returncode = 0
            benders_driver.launch(simulation_output_path)
            assert run_function.call_args.args[0] == expected_cmd

    def test_raise_execution_error(self, tmp_path):

        my_benders_mpi = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        exe_path =  os.path.normpath(os.path.join(my_install_dir, my_benders_mpi))

        if sys.platform.startswith("win32"):
            MPI_LAUNCHER = "mpiexec"
            MPI_N = "-n"
        elif sys.platform.startswith("linux"):
            MPI_LAUNCHER = "mpirun"
            MPI_N = "-np"
        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = True)
        
        benders_driver = BendersDriver(benders_driver_data)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)
        assert benders_driver.get_solver() == my_benders_mpi
        with patch(MOCK_SUBPROCESS_RUN, autospec = True) as run_function :
            with pytest.raises(BendersDriver.BendersExecutionError) :
                expected_cmd = [MPI_LAUNCHER, MPI_N, str(my_n_mpi), exe_path, "options.txt" ]
                benders_driver.launch(simulation_output_path)
                assert run_function.call_args.args[0] == expected_cmd

    # def test_unavailable_solver(self, tmp_path):

    def test_solver_is_mpi_benders(self):
        my_benders_mpi = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        benders_driver = BendersDriver(benders_driver_data)
        assert benders_driver.get_solver() == my_benders_mpi
    def test_solver_is_merge_mps(self):
        my_merge_mps = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "mergeMPS",
                                                            MERGE_MPS  = my_merge_mps,
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        benders_driver = BendersDriver(benders_driver_data)
        assert benders_driver.get_solver() == my_merge_mps
            
    def test_solver_is_sequential(self):
        my_benders_sequential = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "sequential",
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        benders_driver = BendersDriver(benders_driver_data)
        assert benders_driver.get_solver() == my_benders_sequential

    def test_cleaning_log_files_empty_param(self):
        benders_driver = BendersDriver(self.empty_benders_driver_data)
        benders_driver.clean_log_files("")

    def test_cleaning_log_files_good_format(self, tmp_path):
        my_benders_sequential = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "sequential",
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)

        os.chdir(tmp_path)

        accepted_suffixs = ["Log", "Log6565", "Log.txt", "Log.txt.suffix"]
        accepted_log_file_names = [my_benders_sequential+suffix for suffix in accepted_suffixs]
        accepted_log_files = [self._create_empty_file(tmp_path, accepted_log_file_name) for accepted_log_file_name in accepted_log_file_names ]
        
            

        # check that files are created
        for file in accepted_log_files:
            assert file.exists()

        benders_driver = BendersDriver(benders_driver_data)
        benders_driver.clean_log_files(my_benders_sequential)

        # check that files are removed
        for file in accepted_log_files:
            assert not file.exists()

    def test_cleaning_log_files_bad_format(self, tmp_path):
        my_benders_sequential = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "sequential",
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)

        os.chdir(tmp_path)

        accepted_suffixs = ["zfz", "112 e","."]
        accepted_log_file_names = [my_benders_sequential+suffix for suffix in accepted_suffixs]
        accepted_log_files = [self._create_empty_file(tmp_path, accepted_log_file_name) for accepted_log_file_name in accepted_log_file_names ]
        
            

        # check that files are created
        for file in accepted_log_files:
            assert file.exists()

        benders_driver = BendersDriver(benders_driver_data)
        benders_driver.clean_log_files(my_benders_sequential)

        # check that files still exists
        for file in accepted_log_files:
            assert file.exists()

    def test_clean_solver_log_file(self, tmp_path):
        my_benders_sequential = "sequential"
        solver =  "solver_name"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = my_benders_sequential ,
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)
        solver_log_fname = solver +".log"
        solver_log_file = lp_path / solver_log_fname
        solver_log_file.write_text("")

        assert solver_log_file.exists()
        benders_driver = BendersDriver(benders_driver_data)
        benders_driver.clean_log_files(solver)

        assert not solver_log_file.exists()
        
    def test_unsupported_platform(self, tmp_path):
        my_benders_sequential = "sequential"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = my_benders_sequential ,
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)

        
        with patch(MOCK_SYS, autospec=True) as sys_ :
            sys_.platform = "exotic_platform"
            with pytest.raises(BendersDriver.BendersUnsupportedPlatform):
                BendersDriver(benders_driver_data)
        
    def test_mock_linux_platform(self, tmp_path):

        my_benders_mpi = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        exe_path =  os.path.normpath(os.path.join(my_install_dir, my_benders_mpi))

        MPI_LAUNCHER = "mpirun"
        MPI_N = "-np"
        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = True)
        
        with patch(MOCK_SYS, autospec=True) as sys_ :
            sys_.platform = "linux"
            benders_driver = BendersDriver(benders_driver_data)
            assert benders_driver.MPI_LAUNCHER == MPI_LAUNCHER
            assert benders_driver.MPI_N == MPI_N

    def test_clean_benders_step_if_not_keep_mps(self, tmp_path):
        my_benders_mpi = "something"
        my_install_dir = Path("Dummy/Path/to/")
        my_n_mpi = 13
        benders_driver_data  = BendersDriverData(install_dir = my_install_dir,
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = my_n_mpi,
                                                            keep_mps = False)
        
        benders_driver = BendersDriver(benders_driver_data)

        simulation_output_path = tmp_path
        lp_path =  Path(os.path.normpath(os.path.join(simulation_output_path, 'lp')))
        lp_path.mkdir()
        os.chdir(lp_path)

        mps_fnames = ["master.mps", "1.mps","2.mps"]
        mps_files = [self._create_empty_file(lp_path, mps_fname) for mps_fname in mps_fnames ]

        lp_fnames = ["master.lp", "1.lp","2.lp"]
        lp_files = [self._create_empty_file(lp_path, lp_fname) for lp_fname in lp_fnames ]

        benders_files = mps_files+lp_files
        # check that files are created
        for file in benders_files:
            assert file.exists()

        with patch(MOCK_SUBPROCESS_RUN, autospec = True) as run_function :
            run_function.return_value.returncode = 0
            benders_driver.launch(simulation_output_path)

        
        # check that files are deleted
        for file in benders_files:
            assert not file.exists()
        

    def _create_empty_file(self, tmp_path : Path , fname: str):
        file = tmp_path / fname
        file.write_text("") 
        return file