import pytest
from test_bendersEndToEnd import run_solver2


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, allow_run_as_root):
    run_solver2(install_dir, allow_run_as_root, 'BENDERS_MPI')
