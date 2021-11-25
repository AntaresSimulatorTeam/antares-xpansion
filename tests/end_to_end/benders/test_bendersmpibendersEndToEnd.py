import pytest
from test_bendersEndToEnd import run_solver

@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir):
    run_solver(install_dir, 'BENDERS_MPI')
