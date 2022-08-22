import pytest
from test_restartBendersEndToEnd import run_solver


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, tmp_path, allow_run_as_root):
    run_solver(install_dir, 'BENDERS_MPI', tmp_path, allow_run_as_root)
