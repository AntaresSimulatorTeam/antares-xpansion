import pytest
from test_bendersEndToEnd import run_solver2


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, mpi_custom_arguments):
    run_solver2(install_dir, mpi_custom_arguments, 'BENDERS_MPI')
