import pytest
from test_bendersEndToEnd import run_solver


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, allow_run_as_root, tmp_path):
    run_solver(install_dir, 'BENDERS', tmp_path, allow_run_as_root, mpi=True)
