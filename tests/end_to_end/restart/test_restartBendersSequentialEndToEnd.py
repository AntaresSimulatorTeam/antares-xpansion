import pytest
from test_restartBendersEndToEnd import run_solver

## TESTS ##


@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(install_dir):
    run_solver(install_dir, 'BENDERS_SEQUENTIAL')
