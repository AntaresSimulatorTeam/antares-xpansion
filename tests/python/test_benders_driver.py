import pytest
import os, sys
from pathlib import Path

from antares_xpansion.benders_driver import BendersDriver, BendersDriverData, BendersDriverException

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
        a_benders_driver = BendersDriver(self.empty_benders_driver_data)
        
        assert a_benders_driver.install_dir == self.empty_benders_driver_data.install_dir 
        assert a_benders_driver.BENDERS_MPI == self.empty_benders_driver_data.BENDERS_MPI
        assert a_benders_driver.method == self.empty_benders_driver_data.method
        assert a_benders_driver.MERGE_MPS == self.empty_benders_driver_data.MERGE_MPS
        assert a_benders_driver.BENDERS_SEQUENTIAL == self.empty_benders_driver_data.BENDERS_SEQUENTIAL
        assert a_benders_driver.n_mpi == self.empty_benders_driver_data.n_mpi
        assert a_benders_driver.keep_mps == self.empty_benders_driver_data.keep_mps

    def test_driver_with_data(self):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        a_benders_driver = BendersDriver(benders_driver_data)
        
        assert a_benders_driver.install_dir == benders_driver_data.install_dir 
        assert a_benders_driver.BENDERS_MPI == benders_driver_data.BENDERS_MPI
        assert a_benders_driver.method == benders_driver_data.method
        assert a_benders_driver.MERGE_MPS == benders_driver_data.MERGE_MPS
        assert a_benders_driver.BENDERS_SEQUENTIAL == benders_driver_data.BENDERS_SEQUENTIAL
        assert a_benders_driver.n_mpi == benders_driver_data.n_mpi
        assert a_benders_driver.keep_mps == benders_driver_data.keep_mps

    def test_mpi_options(self):
        a_benders_driver = BendersDriver(self.empty_benders_driver_data)

        if sys.platform.startswith("win32"):
            assert a_benders_driver.MPI_LAUNCHER == "mpiexec"
            assert a_benders_driver.MPI_N == "-n"
        elif sys.platform.startswith("linux"):
            a_benders_driver.MPI_LAUNCHER = "mpirun"
            a_benders_driver.MPI_N = "-np"
    
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
        a_benders_driver = BendersDriver(benders_driver_data)
        a_benders_driver.set_simulation_output_path(tmp_path)
        assert a_benders_driver.get_lp_path() == lp_path
    
    def test_empty_output_path(self, tmp_path):
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "test",
                                                            method = "test",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)

        a_benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(BendersDriverException.BendersLp_PathError):
            a_benders_driver.launch(tmp_path)

    
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
        a_benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(SystemExit):
            a_benders_driver.launch(tmp_path)
            
    
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
        a_benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(SystemExit):
            a_benders_driver.launch(tmp_path)
            
    
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
        a_benders_driver = BendersDriver(benders_driver_data)
        with pytest.raises(SystemExit):
            a_benders_driver.launch(tmp_path)
                        
    def test_benders_cmd(self, tmp_path):
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
                                                            keep_mps = False)
        
        a_benders_driver = BendersDriver(benders_driver_data)

        
        assert a_benders_driver.get_solver() == my_benders_mpi
        assert a_benders_driver.get_solver_cmd(my_benders_mpi) == [MPI_LAUNCHER, MPI_N, str(my_n_mpi), exe_path, "options.txt" ]

    def test_solver_is_mpi_benders(self):
        my_benders_mpi = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = my_benders_mpi,
                                                            method = "mpibenders",
                                                            MERGE_MPS  = "test",
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        a_benders_driver = BendersDriver(benders_driver_data)
        assert a_benders_driver.get_solver() == my_benders_mpi
    def test_solver_is_merge_mps(self):
        my_merge_mps = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "mergeMPS",
                                                            MERGE_MPS  = my_merge_mps,
                                                            BENDERS_SEQUENTIAL = "test",
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        a_benders_driver = BendersDriver(benders_driver_data)
        assert a_benders_driver.get_solver() == my_merge_mps
            
    def test_solver_is_sequential(self):
        my_benders_sequential = "something"
        benders_driver_data  = BendersDriverData(install_dir = Path("Dummy/Path/to/"),
                                                            BENDERS_MPI = "",
                                                            method = "sequential",
                                                            MERGE_MPS  = "",
                                                            BENDERS_SEQUENTIAL = my_benders_sequential,
                                                            n_mpi = 13,
                                                            keep_mps = False)
        
        a_benders_driver = BendersDriver(benders_driver_data)
        assert a_benders_driver.get_solver() == my_benders_sequential

    def test_cleaning_log_files_empty_param(self):
        a_benders_driver = BendersDriver(self.empty_benders_driver_data)
        a_benders_driver.clean_log_files("")

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

        a_benders_driver = BendersDriver(benders_driver_data)
        a_benders_driver.clean_log_files(my_benders_sequential)

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

        a_benders_driver = BendersDriver(benders_driver_data)
        a_benders_driver.clean_log_files(my_benders_sequential)

        # check that files are still exists
        for file in accepted_log_files:
            assert file.exists()


    def _create_empty_file(self, tmp_path : Path , fname: str):
        file = tmp_path / fname
        file.write_text("") 
        return file